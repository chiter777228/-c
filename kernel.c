
/*********************************************************************************
 *                                                                               *
 *                                                                               *
 *    Name       : kernel.c                                                      *
 *    Date       : 23-Feb-2014                                                   *
 *    Version    : 0.0.1                                                         *
 *    Source     : C                                                             *
 *    Author     : Ashakiran Bhatter                                             *
 *                                                                               *
 *    Описание: За загрузку этого файла отвечает stage0.bin, который передает    *
 *                 ему право выполнения. Его функциональность                    *
 *                 заключается в отображении экрана-заставки и командной строки. *
 *    Внимание   : Вводить команды бессмысленно, так как они не запрограммированы*                                                                           
 *                                                                               *
 *********************************************************************************/
/* генерирует 16-битный код                                  */
__asm__(".code16\n");
/* переход к основной функции                                */
__asm__("jmpl $0x1000, $main\n");

#define TRUE  0x01
#define FALSE 0x00

char str[] = "$> ";

/* функция установки регистров и стека */
/* параметры: none                     */
void initEnvironment() {
     __asm__ __volatile__(
          "cli;"
          "movw $0x0000, %ax;"
          "movw %ax, %ss;"
          "movw $0xffff, %sp;"
          "cld;"
     );

     __asm__ __volatile__(
          "movw $0x1000, %ax;"
          "movw %ax, %ds;"
          "movw %ax, %es;"
          "movw %ax, %fs;"
          "movw %ax, %gs;"
     );
}

/* VGA-функции. */
/* функция для установки режима VGA на 80*24   */
void setResolution() {
     __asm__ __volatile__(
          "int $0x10" : : "a"(0x0003)
     );
}

/* функция очистки буфера экрана разделяющими пробелами */
void clearScreen() {
     __asm__ __volatile__ (
          "int $0x10" : : "a"(0x0200), "b"(0x0000), "d"(0x0000)
     );
     __asm__ __volatile__ (
          "int $0x10" : : "a"(0x0920), "b"(0x0007), "c"(0x2000)
     );
}

/* функция установки позиции курсора на заданный столбец и строку */

void setCursor(short col, short row) {
     __asm__ __volatile__ (
          "int $0x10" : : "a"(0x0200), "d"((row <<= 8) | col)
     );
}

/* функция включения и отключения курсора */
void showCursor(short choice) {
     if(choice == FALSE) {
          __asm__ __volatile__(
               "int $0x10" : : "a"(0x0100), "c"(0x3200)
          );
     } else {
          __asm__ __volatile__(
               "int $0x10" : : "a"(0x0100), "c"(0x0007)
          );
     }
}

/* функция инициализации режима VGA на 80*25,            */
/* очистки экрана и установки положения курсора на (0,0) */
void initVGA() {
     setResolution();
     clearScreen();
     setCursor(0, 0);
}

/* I/O-функции. */
/* функция для получения символа с клавиатуры без эха*/
void getch() {
     __asm__ __volatile__ (
          "xorw %ax, %ax\n"
          "int $0x16\n"
     );
}

/* эта функция аналогична getch(),                                 */
/* но возвращает скан-код клавиши и соответствующее значение ascii */
short getchar() {
     short word;

     __asm__ __volatile__(
          "int $0x16" : : "a"(0x1000)
     );

     __asm__ __volatile__(
          "movw %%ax, %0" : "=r"(word)
     );

     return word;
}

/* функция для отображения нажатых клавиш на экране*/
void putchar(short ch) {
     __asm__ __volatile__(
          "int $0x10" : : "a"(0x0e00 | (char)ch)
     );
}

/* функция вывода на экран строки с завершающим нулем */
void printString(const char* pStr) {
     while(*pStr) {
          __asm__ __volatile__ (
               "int $0x10" : : "a"(0x0e00 | *pStr), "b"(0x0002)
          );
          ++pStr;
     }
}

/* функция, вызывающая задержку на несколько секунд */
void delay(int seconds) {
     __asm__ __volatile__(
          "int $0x15" : : "a"(0x8600), "c"(0x000f * seconds), "d"(0x4240 * seconds)
     );
}

/* Строковая функция. */
/* эта функция вычисляет длину строки и возвращает ее */
int strlength(const char* pStr) {
     int i = 0;

     while(*pStr) {
          ++i;
     }
     return i;
}

/* Функция UI. */
/*эта функция отображает логотип */
void splashScreen(const char* pStr) {
     showCursor(FALSE);
     clearScreen();
     setCursor(0, 9);
     printString(pStr);
     delay(10);
}

/* Оболочка. */
/* функция для отображения фиктивной командной строки.                  */
/* При нажатии клавиши Ввод выполняется переход на следующую строку     */
void shell() {
     clearScreen();
     showCursor(TRUE);
     while(TRUE) {
          printString(str);
          short byte;
          while((byte = getchar())) {
               if((byte >> 8)  == 0x1c) {
                    putchar(10);
                    putchar(13);
                    break;
               } else {
                    putchar(byte);
               }
          }
     }
}

/* точка входа в ядро */
void main() {
     const char msgPicture[] = 
             "                     ..                                              \n\r"
             "                      ++`                                            \n\r"
             "                       :ho.        `.-/++/.                          \n\r"
             "                        `/hh+.         ``:sds:                       \n\r"
             "                          `-odds/-`        .MNd/`                    \n\r"
             "                             `.+ydmdyo/:--/yMMMMd/                   \n\r"
             "                                `:+hMMMNNNMMMddNMMh:`                \n\r"
             "                   `-:/+++/:-:ohmNMMMMMMMMMMMm+-+mMNd`               \n\r"
             "                `-+oo+osdMMMNMMMMMMMMMMMMMMMMMMNmNMMM/`              \n\r"
             "                ```   .+mMMMMMMMMMMMMMMMMMMMMMMMMMMMMNmho:.`         \n\r"
             "                    `omMMMMMMMMMMMMMMMMMMNMdydMMdNMMMMMMMMdo+-       \n\r"
             "                .:oymMMMMMMMMMMMMMNdo/hMMd+ds-:h/-yMdydMNdNdNN+      \n\r"
             "              -oosdMMMMMMMMMMMMMMd:`  `yMM+.+h+.-  /y `/m.:mmmN      \n\r"
             "             -:`  dMMMMMMMMMMMMMd.     `mMNo..+y/`  .   .  -/.s      \n\r"
             "             `   -MMMMMMMMMMMMMM-       -mMMmo-./s/.`         `      \n\r"
             "                `+MMMMMMMMMMMMMM-        .smMy:.``-+oo+//:-.`        \n\r"
             "               .yNMMMMMMMMMMMMMMd.         .+dmh+:.  `-::/+:.        \n\r"
             "               y+-mMMMMMMMMMMMMMMm/`          ./o+-`       .         \n\r"
             "              :-  :MMMMMMMMMMMMMMMMmy/.`                             \n\r"
             "              `   `hMMMMMMMMMMMMMMMMMMNds/.`                         \n\r"
             "                  sNhNMMMMMMMMMMMMMMMMMMMMNh+.                       \n\r"
             "                 -d. :mMMMMMMMMMMMMMMMMMMMMMMNh:`                    \n\r"
             "                 /.   .hMMMMMMMMMMMMMMMMMMMMMMMMh.                   \n\r"
             "                 .     `sMMMMMMMMMMMMMMMMMMMMMMMMN.                  \n\r"
             "                         hMMMMMMMMMMMMMMMMMMMMMMMMy                  \n\r"
             "                         +MMMMMMMMMMMMMMMMMMMMMMMMh                      ";
     const char msgWelcome[] = 
             "              *******************************************************\n\r"
             "              *                                                     *\n\r"
             "              *        Welcome to kirUX Operating System            *\n\r"
             "              *                                                     *\n\r"
             "              *******************************************************\n\r"
             "              *                                                     *\n\r" 
             "              *                                                     *\n\r"
             "              *        Author : Ashakiran Bhatter                   *\n\r"
             "              *        Version: 0.0.1                               *\n\r"
             "              *        Date   : 01-Mar-2014                         *\n\r"
             "              *                                                     *\n\r"
             "              ******************************************************";
     initEnvironment(); 
     initVGA();
     splashScreen(msgPicture);
     splashScreen(msgWelcome);

     shell(); 

     while(1);
}


