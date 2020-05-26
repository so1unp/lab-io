// Simple PIO-based (non-DMA) IDE driver code.
// Con comentarios extras (ver archvo IDE-reference.txt)

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "buf.h"

// Indicates the drive is preparing to send/receive data (wait for it 
// to clear). In case of 'hang' (it never clears), do a software reset.
#define IDE_BSY       0x80

// Bit is clear when drive is spun down, or after an error. Set otherwise. 
#define IDE_DRDY      0x40 

// Drive Fault Error (does not set ERR).
#define IDE_DF        0x20

// Indicates an error occurred. Send a new command to clear it (or 
// nuke it with a Software Reset).
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1; // if 1, there is second hard-drive
static void idestart(struct buf*);

// Wait for IDE disk to become ready.
static int
idewait(int checkerr)
{
  int r;
  
  /*
   * 0x1f7 es el puerto de comandos de la controladora IDE, que
   * permite enviar comandos o leer el estado actual del disco.
   * Con inb(0x1f7) se lee el estado del disco (status register).
   * 
   * Luego, se hace un and (&) entre el valor del status register,
   * guardado en r, y la bandera IDE_BSY | IDE_DRDY. Mientras el
   * valor sea distinto de IDE_DRDY, se mantiene en una espera
   * activa.
   */
  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
    ;
  
  /*
   * (r & (IDE_DF|IDE_ERR)) es un valor distinto de cero si ocurrio
   * algún error en la controladora. Si checkerr > 0, entonces se 
   * retorna -1 en caso de error.
   */
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return -1;  
  
  return 0;
}

void
ideinit(void)
{
  int i;

  initlock(&idelock, "ide");       // initialize a mutual exclusion spinlock called "ide"
  picenable(IRQ_IDE);
  ioapicenable(IRQ_IDE, ncpu - 1);
  
  idewait(0);                      // wait for the hard-disk drive to be ready
  
  // Check if disk 1 is present
  /*
   * 0x1f6 es el registro selectord de disco / cabezal
   * El bit 4 en el registro indica el disco sobre el 
   * que operar. Si es 0, es el disco rigido primario, 
   * y si es 1, selecciona el disco secundario. Con la
   * expresión (1<<4), se pone en 1 el bit 4.
   */  
  outb(0x1f6, 0xe0 | (1<<4));
  
  /*
   * Verifica repetidamente el valor del registro de 
   * estado (0x1f7), hasta que sea distinto de cero. Si 
   * es el caso, es que hay un disco secundario presente.
   */
  for(i=0; i<1000; i++){
    if(inb(0x1f7) != 0){
      havedisk1 = 1;
      break;
    }
  }
  
  // Switch back to disk 0.
  outb(0x1f6, 0xe0 | (0<<4));
}

// Start the request for b.  Caller must hold idelock.
static void
idestart(struct buf *b)
{
  if(b == 0)
    panic("idestart");  // se invoco idestart() con un puntero inválido

  // espera a que el disco este listo
  idewait(0);
  
  /*
   * Generate interrupt. If 1, stop the current device from sending interrupts.
   */
  outb(0x3f6, 0);  

  /*
   * 0x1f2: SECTOR COUNT REGISTER
   * Defines the number of sectors of data to be transferred across 
   * the host bus, for the subsequent command.   
   */
  outb(0x1f2, 1);
  
  /*
   * 0x1f3: SECTOR NUMBER REGISTER
   * Contains the ID number of the first sector to be accessed by the
   * subsequent command. The sector can be from one to the maximum 
   * number of sectors per track.
   */
  outb(0x1f3, b->sector & 0xff);
  
  /* 0x1f4: CYLINDER LOW REGISTER
   * Contains the eight low order bits of the starting cylinder 
   * address for any disk access.
   */
  outb(0x1f4, (b->sector >> 8) & 0xff);
  
  /* 0x1f5: CYLINDER HIGH REGISTER
   * Contains the eight high order bits of the starting cylinder 
   * address for any disk access.
   */
  outb(0x1f5, (b->sector >> 16) & 0xff);
  
  /* 0x1f6: DRIVE/HEAD REGISTER
   * Contains the drive ID number and its head number for any disk 
   * access.
   * e0: ‭11100000‬ (estos bits ya estan seteados por defecto)
   * (b->dev & 1) << 4 (el disco del cual leer/escribir)
   * (b->sector >> 24) & 0x0f (el numero de sector)
   */
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((b->sector>>24)&0x0f));
  
  if(b->flags & B_DIRTY)  // buffer needs to be written to disk?
  {
    // 0x1f7: COMMAND REGISTER (write)
    // Write IDE_CMD_WRITE command in 0x1f7 port.
    outb(0x1f7, IDE_CMD_WRITE);
    
    // 0x1f70 DATA PORT REGISTER
    // All data transferred between the device data buffer and the host
    // passes through this register.
    outsl(0x1f0, b->data, BSIZE/4);
  } 
  else 
  {
    // Write IDE_CMD_READ command in 0x1f7 port.
    outb(0x1f7, IDE_CMD_READ);
  }
}

// Interrupt handler.
void
ideintr(void)
{
  struct buf *b;

  // acquire the mutual exclusion spinlock "idelock"
  acquire(&idelock);
  
  // Take first buffer off queue.
  if((b = idequeue) == 0){
    release(&idelock);
    cprintf("Spurious IDE interrupt.\n");
    return;
  }
  
  // b->qnext is now the new head of the list
  idequeue = b->qnext;

  // Read data if needed.
  // 0x1f0 DATA PORT REGISTER
  // All data transferred between the device data buffer and the host
  // passes through this register.
  if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
    insl(0x1f0, b->data, BSIZE/4);
  
  // Wake process waiting for this buf.
  b->flags |= B_VALID;  // set that this buffer has been read from the disk
  b->flags &= ~B_DIRTY; // clear dirty bit in flags (we have fresh bytes from the hard-disk!)
  wakeup(b);            // wake-up the process waiting for buffer b (see proc.c)
  
  // Start disk on next buf in queue.
  if(idequeue != 0)
    idestart(idequeue);  // process next buffer

  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk. 
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void
iderw(struct buf *b)
{
  struct buf **pp;

  if(!(b->flags & B_BUSY))
    panic("iderw: buf not busy");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if(b->dev != 0 && !havedisk1)
    panic("idrw: ide disk 1 not present");

  // acquire the mutual exclusion spinlock "idelock"
  acquire(&idelock);

  // Append b to idequeue.
  b->qnext = 0;
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)
    ;
  *pp = b;
  
  // Start disk if b is the only buffer in idequeue
  if(idequeue == b)
    idestart(b); // start the request -- when finished, a interrupt will be generated, and processed by ideintr()
  
  // Wait for request to finish.
  // Assuming will not sleep too long: ignore cp->killed.
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID)
    // Releases idelock and put the process to sleep. Then reacquires lock 
    // when reawakened. See proc.c for details.
    sleep(b, &idelock);

  release(&idelock); // release idelock, so other process can access the drive
}
