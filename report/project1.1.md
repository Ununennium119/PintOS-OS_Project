# تمرین گروهی ۱.۱ - گزارش

## گروه ۲

## نام و آدرس پست الکترونیکی اعضای گروه

محمد مهدی صادقی moh02sadeghi@gmail.com

آرش یادگاری arashyadegari0402@gmail.com

محمد خسروی mohamad138121@gmail.com

عرفان مجیبی mojibierfan@gmail.com

## توضیحات کلی پیاده سازی

### پاس دادن آرگومان‌ها

در تابع load
پردازش بر روی استرینگ file_name انجام می‌شود و این رشته توکینایز می‌شود و در نتیجه
مقدار argc و آرایه argv که مورد نیاز ما هستند مشخص می‌شوند.
سپس این مقادیر به تابع setup_stack
پاس داده می‌شوند که وظیفه پوش کردن آنها در استک را انجام دهد تا توسط برنامه در حال اجرا خوانده شوند.

توجه کنید ابتدا خود رشته‌ها در استک پوش می‌شوند، سپس آدرس آنها پوش می‌شود، و در انتها argc و argv که ورودی‌های برنامه هستند روی آنها پوش می‌شوند.

### سیستم‌کال‌های پردازه‌ها

**practice**

صرفا آرگومان را یکی زیاد می‌کند و بر‌می‌گرداند.

**halt**

صرفا ```shutdown_power_off``` را فراخوانی می‌کند.

**wait**

استراکت مربوط به child را با استفاده از tid داده شده پیدا می‌کند و روی سمافور wait_sema تابع sema_down را کال می‌کند. بعد از خروج از تابع sema_down مقدار exit code پراسس child برگردانده می‌شود.

**exec**

استراکت thread_details را می‌سازد و به آن مقدار اولیه می‌دهد. ورودی‌های لازم را در استراکت thread_args ذخیره می‌کند و آن را به تابع thread_create پاس می‌دهد تا به عنوان ورودی به تابع start_process داده شود. سپس روی سمافور wait_sema تابع sema_down را کال می‌کند. وقتی کار ساختن thread تمام شد از تابع sema_down بیرون می‌آید و موفقیت عملیات را بررسی می‌کند و با توجه به آن tid یا ۱- را برمی‌گرداند.

**exit**

متغیر reference_count مربوط به threadهای فرزند و خودش را یک واحد کاهش می‌دهد و سپس با بررسی همین متغیر تصمیم می‌گیرد که ریسورس‌های مربوط را free کند. در نهایت مقدار exit code پراسس را برمی‌گرداند.

### سیستم‌کال‌های فایل‌ها

**create**

ابتدا معتبر بودن آرگومان‌ها (آدرس و طول اسم فایل)
بررسی می‌شود و در صورتی که درست باشد با استفاده از تابع
```filesys_create```
فایل ساخته می‌شود.

**open**

ابتدا معتبر بودن آرگومان‌ها (آدرس و طول اسم فایل)
بررسی می‌شود و در صورتی که درست باشد با استفاده از تابع
```filesys_open```
فایل باز می‌شود.
سپس به فایل باز شده یک fd تخصیص داده می‌شود که ریترن می‌شود.

**read**

پس از بررسی معتبر بودن آدرس بافر و اینکه فایلی که قرار است خوانده شود stdout نباشد،
اگر stdin باشد کاراکتر به کاراکتر میخواند و در بافر ذخیره می‌کند.
در غیر این صورت هم پس از بررسی اینکه توصیف‌کننده فایل واقعا وجود دارد یا خیر، فایل را می‌خواند و تعداد بایت‌های خوانده شده را در رجیستر eax ذخیره می‌کند.

**write**

پس از چک‌کردن‌های مشابه `read`، اگر stdout باشد با استفاده از `putbuf` مقادیر را در بافر قرار می‌دهد و در غیر این صورت
با استفاده از `file_write` مقادیر را در فایل ذخیره می‌کند و تعداد بایت‌های نوشته شده را در رجیستر `eax` ذخیره می‌کند.

**close**

ابتدا ریسه کنونی را دریافت کرده و سپس بررسی می‌کنیم که ```file descriptor``` معتبر است یا خیر. معتبر بودن ```file descriptor``` به این معنا است که فایلی که به آن اشاره می‌کند، باز باشد و مقدار خود آن نیز معتبر باشد. در صورت معتبر نبودن مقدار معادل با خطا برگرندانده می‌شود و در صورت معتبر بودن مقدار اشاره‌گر به فایل را به -۱ تغییر می‌دهیم. این بدین معنا است که این ```file descriptor``` آزاد است و می‌توان از آن برای باز کردن یک فایل دیگر استفاده کرد.
در نهایت در صورت موفقیت آمیز بودن عملیات مقدار ۱ در ثبات ‍‍```eax``` ذخیره‌ می‌شود.

**filesize**

پس از چک‌کردن اینکه fd واقعا وجود دارد، با استفاده از `file_length` اندازه فایل را پیدا کرده و در  `eax` ذخیره می‌کنیم.

**seek**

این دستور به ما امکان می‌دهد تا با مشخص کردن یک ```position``` و یک ```file descriptor``` مقادیر مدنظر را در یک موقیعت خاص بنویسیم و یا از آن مکان بخوانیم.

برای انجام این دستور از تابع آماده ```file_seek``` در ```file.c``` استفاده می‌کنیم. این تابع در ابتدا بررسی می‌کند که فایل داده شده وجود داشته باشد و سپس در صورتی که آدرس داده شده یعنی ```position > 0``` باشد آنگاه مقدار ```pos``` را در استراکت ```file.c``` عوض می‌کند.

**tell**

این دستور به ما این امکان را می‌دهد تا موقیت کنونی خوانش و یا نوشته شدن بر روی یک فایل را بدست آوریم. قابل ذکر است که این موقعیت نسبت به اول فایل سنجیده می‌شود. در صورت معتبر بودن آدرس فایل، خروجی ۱ در ثبات ```eax``` ذخیره می‌شود.

برای انجام این دستور از تابع آماده ```file_tell``` در ```file.c``` استفاده می‌کنیم. این تابع در ابتدا بررسی می‌کند که فایل داده شده وجود داشته باشد و سپس در صورت وجود فایل مقدار ```pos``` را از استراکت ```file``` باز می‌گرداند.

## تغییرات نسبت به سند طراحی

## وظایف افراد

همه اعضای گروه در جریان پیاده‌سازی بخش‌های مختلف بودند و جلسات زیادی از پیاده‌سازی به صورت چهار نفره برگزار شد، اما بعضی بخش‌ها نیز به صورت جداگانه بر عهده افراد قرار گرفت تا انجام شود.