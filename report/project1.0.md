# تمرین گروهی ۱/۰ - آشنایی با pintos

## شماره گروه ۲

> نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

محمد مهدی صادقی moh02sadeghi@gmail.com

آرش یادگاری arashyadegari0402@gmail.com

محمد خسروی mohamad138121@gmail.com

عرفان مجیبی mojibierfan@gmail.com

## مقدمات


> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت بنویسید.

\-

> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع  درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

\-

## آشنایی با pintos

در مستند تمرین گروهی ۱۹ سوال مطرح شده است. پاسخ آن ها را در زیر بنویسید.

### یافتن دستور معیوب

۱.
0xc0000008

۲.
0x8048757

۳.
command:

```objdump -d  do-nothing```

function name: `_start`

instruction: `mov    0x24(%esp),%eax`

۴.

```C
void
_start (int argc, char *argv[])
{
  exit (main (argc, argv));
}
```

`sub    $0x1c,%esp` : allocate 28 bytes from stack

`mov    0x24(%esp),%eax`: move the value at address [%esp] + 36 to %eax

`mov    %eax,0x4(%esp)`: move %eax to 4 + [%esp]

`mov    0x20(%esp),%eax`:  move the value at address [%esp] + 32 to %eax

`mov    %eax,(%esp)`: move %eax to [%esp]

`call   80480a0 <main>`: call main

`mov    %eax,(%esp)`: move %eax to [%esp]

`call   804a2bc <exit>`: call exit

در واقع پارامترهای تابع را در استک پوش می‌کند و با مقدار خروجی تابع main تابع exit را صدا می‌زند.

۵.
هدف خواندن ورودی‌های تابع مذکور بوده است، که در استک وجود ندارد و باعث شده که به قسمتی از حافظه اشاره کند که برای استک نیست.

### به سوی crash

۶.
name = `main`

address: 0xc000e000

other thread: `idle`

```C
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000edec <incomplete sequence \357>, priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۷.

```C
#0  process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
#1  0xc0020268 in run_task (argv=0xc00357cc <argv+12>) at ../../threads/init.c:288
#2  0xc0020921 in run_actions (argv=0xc00357cc <argv+12>) at ../../threads/init.c:340
#3  main () at ../../threads/init.c:133
```

```C
init.c:133: run_actions (argv);
```

***

```C
init.c:340: a->function (argv);
```

***

```C
init.c:288: process_wait (process_execute (task));
```

۸.

```C
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_BLOCKED, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0037314 <temporary+4>, next = 0xc003731c <temporary+12>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc010a020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #2: 0xc010a000 {tid = 3, status = THREAD_RUNNING, name = "do-nothing\000\000\000\000\000", stack = 0xc010afd4 "", priority = 31, allelem = {prev = 0xc0104020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۹.

process.c:45:

```C
tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
```

۱۰.

```C
$2 = {edi = 0x0, esi = 0x0, ebp = 0x0, esp_dummy = 0x0, ebx = 0x0, edx = 0x0, ecx = 0x0, eax = 0x0, gs = 0x23, fs = 0x23, es = 0x23, ds = 0x23, vec_no = 0x0,
 error_code = 0x0, frame_pointer = 0x0, eip = 0x8048754, cs = 0x1b, eflags = 0x202, esp = 0xc0000000, ss = 0x23}
```

۱۱.

در این حالت تابع intr_exit اجرا می‌شود که برای خروج از interupt استفاده می‌شود و باید در حالت kernel اجرا شود. 
اما هدف اصلی از این کار انجام context switch است تا در هنگام برگشتن از حالت kernel به حالت user، وضعیت پردازه‌ی جدید (که در if_ تنظیم شده است) در cpu لود شود.

۱۲.

```asm
eax            0x0      0
ecx            0x0      0
edx            0x0      0
ebx            0x0      0
esp            0xc0000000       0xc0000000
ebp            0x0      0x0
esi            0x0      0
edi            0x0      0
eip            0x8048754        0x8048754
eflags         0x202    [ IF ]
cs             0x1b     27
ss             0x23     35
ds             0x23     35
es             0x23     35
fs             0x23     35
gs             0x23     35
```

۱۳.

```C
#0  _start (argc=<unavailable>, argv=<unavailable>) at ../../lib/user/entry.c:9
```

## دیباگ

۱۴.

after calling `load` in `start_process` we add:

```C
if_.esp -= 20;
```

۱۵.

perl
do-stack-align: exit(12)

۱۶.

0xbfffff98:     0x00000001      0x000000a2

۱۷.

`print args[0]`= 1
`print args[1]`= 162

دو مقدار بالای پشته برابر این دو مفدار هستند.

۱۸.

در ابتدای برنامه مقدار آن ۰ است. در init.c ما تابع  process_execute را فراخوانی می‌کنیم که شماره پراسس را به ما برمی‌گرداند و ما آن را به  process_wait می‌دهیم تا تا انتهای آن صبر کند. و حالا تا وقتی که process_exit اجرا نشده باشد مقدار آن به ۱ نمی‌رسد و sema_down اجرا نخواهد شد.

در تابع process_wait کال شده است.

۱۹.

name = `main`

address = `0xc000e000`

other thread = `idle`
