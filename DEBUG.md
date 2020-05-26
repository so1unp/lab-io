# Debugging xv6

Para realizar los ejercicios del laboratorio, resulta muy útil poder ejecutar paso a paso *xv6*. Para esto, vamos a utilizar el debugger *gdb*.

## Número de CPUs
Para que sea más sencillo la ejecución paso a paso del sistema, es aconsejable que qemu simule un sistema con un único procesador. Hay dos maneras de hacerlo. La primera, es modificar en el Makefile la variable `CPUS`, para que siempre se utilice un único procesador:
```
CPUS := 1
```
Como alternativa, se puede indicar el número de procesadores al ejecutar qemu:
```
$ make qemu-nox-gdb CPUS=1
```

## Configurar gdb
Suponiendo que xv6 este en el directorio `/home/usuario/xv6`, tiene que agregar ese *path* en el archivo `.gdbinit`.

El siguiente comando lo hace creando el archivo `.gdbinit` si no existiera:
```
$ echo "add-auto-load-safe-path /home/usuario/xv6" >> ~/.gdbinit
```

## Ejecutar qemu con gdb
Vamos a necesitar trabajar desde dos terminales, en una se ejecuta xv6, en la otra gdb. En ambas terminales el directorio de trabajo tiene que ser donde este instalado xv6.

En una terminal ejecutamos:
```
$ make qemu-nox-gdb CPUS=1
...
*** Now run 'gdb'.
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512  -S -gdb tcp::26000
```

Ahora, en la otra terminal ejecutamos gdb, y nos tiene que aparecer algo así:
```
$ gdb
...
+ target remote localhost:26000
warning: No executable has been specified and target does not support determining executable automatically.  Try using the "file" command.
The target architecture is assumed to be i8086
[f000:fff0]    0xffff0: ljmp   $0x3630,$0xf000e05b
0x0000fff0 in ?? ()
+ symbol-file kernel
warning: A handler for the OS ABI "GNU/Linux" is not built into this configuration of GDB.  Attempting to continue with the default i8086 settings.

(gdb)
```

Si todo funciono correctamente, gdb se queda a la espera de ordenes. Si escribimos `c` (*continue*) y damos enter, debemos ver como en la otra terminal se reanuda la ejecución de xv6.

Si, estando en la terminal que ejecuta gdb, presionamos ^C, gdb interrumpe la ejecución de xv6, indicando donde quedo detenido, y espera por ordenes:
```
(gdb) c
Continuing.
^C
Program received signal SIGINT, Interrupt.
The target architecture is assumed to be i386
=> 0x80103811 <mycpu+17>:       mov    0x80112d00,%esi
mycpu () at proc.c:48
48        for (i = 0; i < ncpu; ++i) {
(gdb)
```

En este caso, interrumpimos la ejecución durante la función `mycpu()` en el archivo `proc.c`.

Podemos ejecutar luego paso a paso con `s` (*step*) o `n` (*next*). La diferencia entre ambas es que `n` no entra en las funciones, y `s` sí. Pueden ver la documentación ejecutando `help`.

También es interesante el comando `list`, que presenta fragmento del codigo fuente antes y despues de la actual sentencia:
```
(gdb) list
43        panic("mycpu called with interrupts enabled\n");
44
45        apicid = lapicid();
46        // APIC IDs are not guaranteed to be contiguous. Maybe we should have
47        // a reverse map, or reserve a register to store &cpus[i].
48        for (i = 0; i < ncpu; ++i) {
49          if (cpus[i].apicid == apicid)
50            return &cpus[i];
51        }
52        panic("unknown apicid\n");
(gdb)
```

## Breakpoints
Mediante el comando `break` podemos crear un breakpoint en una función o en una línea de un archivo fuente. Por ejemplo:
```
break proc.c:51
break idestart()
```

## Mas info...
Hay muchos tutoriales en la red sobre gdb, como por ejemplo [este](http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html).

También pueden consultar la [documentación oficial](https://www.gnu.org/software/gdb/documentation/).
