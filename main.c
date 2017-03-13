#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

static volatile unsigned char heart[9];
static unsigned char column=0;
static unsigned char i,ii,j;

// наборы состояния матрицы
unsigned char KONTUR[9] =     { 0x0e, 0x11, 0x21, 0x41, 0x82, 0x41, 0x21, 0x11, 0x0e };
unsigned char MIN_HEART[9] =  { 0x00, 0x00, 0x00, 0x08, 0x10, 0x08, 0x00, 0x00, 0x00 };
unsigned char MID_HEART[9] =  { 0x00, 0x00, 0x0c, 0x1c, 0x38, 0x1c, 0x0c, 0x00, 0x00 };
unsigned char BIG_HEART[9] =  { 0x00, 0x0e, 0x1e, 0x3e, 0x7c, 0x3e, 0x1e, 0x0e, 0x00 };
unsigned char FULL_HEART[9] = { 0x0e, 0x1f, 0x3f, 0x7f, 0xfe, 0x7f, 0x3f, 0x1f, 0x0e };
unsigned char N_LETTER[9] =   { 0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x3e, 0x00, 0x00 };
unsigned char I_LETTER[9] =   { 0x00, 0x00, 0x3e, 0x10, 0x08, 0x04, 0x3e, 0x00, 0x00 };
unsigned char A_LETTER[9] =   { 0x00, 0x00, 0x3c, 0x12, 0x12, 0x12, 0x3c, 0x00, 0x00 };

// переключаем байт в состояние включено
void led_on (unsigned char x, unsigned char y) 
{
	heart[x]|=1<<y;
}

// переключаем байт в состояние выключено
void led_off (unsigned char x, unsigned char y) {
	heart[x]&=~(1<<y);
}

// выключение всех светодиодов
void clear_displ() {
	PORTD = 0x00;
	for(i = 0; i<9; ++i)
		heart[i]=0x00;
}

// эффект "плавное затухание"
void effect_plavno_down () {
  OCR2=0xff;
  TIMSK=(1<<OCIE2) | (1<<TOIE2);
  while (OCR2!=0) {
    --OCR2;
    _delay_ms(8);
  }
  _delay_ms(500);
  clear_displ();
  _delay_ms(2000);
  TIMSK=(1<<TOIE2);
}

// эффект "Подготовка" (несколько раз моргает центральный светодиод)
void effect_start () {
	led_on (4,3);	_delay_ms(500);
	led_off (4,3);	_delay_ms(250);
	led_on (4,3);	_delay_ms(500);
	led_off (4,3);	_delay_ms(250);
	led_on (4,3);	_delay_ms(500);
	led_off (4,3);	_delay_ms(1000);
}

// эффект "контур" (загораются светодиоды только по контуру)
void effect_kontur() {
	for (i = 0; i < 9; ++i)
		heart[i] = KONTUR[i];
	_delay_ms(2000);
}

// эффект "горизонтальные линии" (горизонтальные линии падают как в тетрисе)
void effect_hor_line() {
	for (i = 0; i <= 7; ++i)
		for (ii = 0; ii <= 7 - i; ++ii) {
			for (j = 0; j <= 8; ++j)
				led_on (j,ii);
			_delay_ms(50);
			if (ii < 7 - i)
				for (j = 0; j <= 8; ++j)
					led_off(j,ii);
		}
	_delay_ms(1000);
	for (ii = 0; ii <= 7; ++ii) {
		for (j = 0; j <= 8; ++j)
			led_off (j,ii);
		_delay_ms(50);
	}
}

// эффект "вертикальные линии" (вертикальные  линии сдвигаются как жалюзи)
void effect_vert_line() {
	for (i = 0; i <= 4; ++i)
		for (ii = 0; ii <= 4 - i; ++ii) {
			for (j = 0; j <= 7; ++j) {
				led_on (ii,j);
				led_on (8-ii,j);
			}
			_delay_ms(50);
			if (ii < 4 - i)
				for (j = 0; j <= 7; ++j) {
					led_off(ii,j);
					led_off(8-ii,j);
				}
		}
	_delay_ms(1000);
	for (ii = 0; ii <= 4; ++ii) {
		for (j = 0; j <= 7; ++j) {
			led_off (4-ii,j);
			led_off (4+ii,j);
		}
		_delay_ms(50);
	}
}

// эффект "буква" (постепенно загорается все сердце, а когда тухнет, остается буква)
void effect_heart_letter(unsigned char *letter) {
  for (i = 0; i < 9; ++i)
    heart[i] = FULL_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | BIG_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | MID_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | MIN_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i];
  _delay_ms(1000);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | MIN_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | MID_HEART[i];
  _delay_ms(100);
  for (i = 0; i < 9; ++i)
    heart[i] = letter[i] | BIG_HEART[i];
  _delay_ms(100);
}

// эффект "имя" (вызывается эффект "буква" для разного набора букв)
void effect_name() {
  effect_heart_letter(N_LETTER);
  effect_heart_letter(I_LETTER);
  effect_heart_letter(N_LETTER);
  effect_heart_letter(A_LETTER);
}

// эффект "стук сердца"
void effect_tuk(unsigned char num) {
	for (ii=0;ii<num;++ii) {
/*		for (i = 0; i < 9; ++i)
			heart[i] = MIN_HEART[i];
		_delay_ms(100);*/
		for (i = 0; i < 9; ++i)
			heart[i] = MID_HEART[i];
		_delay_ms(100);
		for (i = 0; i < 9; ++i)
			heart[i] = BIG_HEART[i];
		_delay_ms(50);
		for (i = 0; i < 9; ++i)
			heart[i] = MID_HEART[i];
		_delay_ms(100);
/*		for (i = 0; i < 9; ++i)
			heart[i] = BIG_HEART[i];
		_delay_ms(100);*/
		for (i = 0; i < 9; ++i)
			heart[i] = FULL_HEART[i];
		_delay_ms(300);
	}
	_delay_ms(2000);
}

// основная программа
void main(void) {
	DDRB = PORTB = 0x3e;
	DDRC = PORTC = 0x0f;
	DDRD = PORTD = 0xff;

	TCCR2 |= (0<<CS22)|(0<<CS21)|(1<<CS20);		// настраиваем делитель
	TIMSK |= (1<<TOIE2);						// разрешаем прерывание по переполнению таймера
//	TCNT0 = 0x00;								// выставляем начальное значение TCNT1
	sei();										// выставляем бит общего разрешения прерываний

	clear_displ();
	effect_start();
	while (1) {
		clear_displ();
		effect_hor_line();
		_delay_ms(250);
//		clear_displ();
		effect_kontur();
		clear_displ();
		effect_vert_line();
		_delay_ms(250);
//		clear_displ();
//		effect_kontur();
//		_delay_ms(250);
//		clear_displ();
		effect_name();
//		clear_displ();
		effect_tuk(10);
		effect_plavno_down();
	}
}

ISR(TIMER2_OVF_vect) {
//	TCNT0 = 0x00;								//выставляем начальное значение TCNT1

	// включаем все столбцы по очереди
	if (column<5) {
		// до 4-го на одном порту
		PORTB = 1<<(column+1);
		PORTC = 0;
	} else {
		// начиная с 4-го на другом
		PORTB = 0;
		PORTC = 1<<(column-5);
	}
	// 3-им портом управляем строками для каждого столбца
	PORTD = heart[column];
	column++;

	// обнуляем счетчик столбцов
	if (column == 9) {
		column = 0;
	}
}
