#include "global.h"

#define SEGMENT_TIME 500

//세븐세그먼트 관련 변수 선언, 초기화
BYTE SegDigit = 0;
WORD FndData = 0;

//가위, 바위, 보 배열
int *choice[]={"ROCK","PAP ","SCI "};

//컴퓨터와 나의 가위바위보 반환 숫자 저장할 변수 선언, 초기화
static int com = 0;
static int you = 0;

//입력할 포트 KEY 정의
#define NOKEY 0
#define KEY7 7
#define KEY6 6
#define KEY5 5
#define KEY4 4

//while문 조건식 변수 선언, 초기화
BYTE gate=1;

//포트 KEY를 저장할 변수 선언, 초기화
BYTE KeyNo = NOKEY;

//세븐세그먼트에 숫자 표시
BYTE code FND[10] ={  ~(0x01|0x02|0x04|0x08|0x10|0x20),    // 0
	~(0x02|0x04),                                     // 1   
	~(0x01|0x02|0x40|0x10|0x08),                     // 2
	~(0x01|0x02|0x40|0x04|0x08),                    // 3
	~(0x20|0x40|0x02|0x04),                         // 4
	~(0x01|0x20|0x40|0x04|0x08),                   // 5
	~(0x20|0x10|0x40|0x04|0x08),                  // 6
	~(0x20|0x01|0x02|0x04),                       // 7
	~(0x01|0x02|0x04|0x40|0x08|0x10|0x20),       // 8
	~(0x20|0x01|0x02|0x40|0x04),                 // 9
};

//딜레이 함수 선언
void ShortDelay(BYTE i) {
	while(i--);    // 7us delay   about 7 Cycle
}
void LongDelay(WORD i) {
	while(i--) {
		ShortDelay(145);  // 1ms delay 
	}
}

//LCD 화면 커서 위치, 글자 표시 함수 선언
void lcd_command_write(BYTE command) {
	LCD_WR_COMMAND = command;
	LongDelay(1);
}
void lcd_char_display(BYTE character) {
	LCD_WR_DATA = character;
	LongDelay(1);
}

//포트, 인터럽트 선언
void Initial_CPU(void) {
	P0 = 0xff;
	P1 = 0xff;
	P2 = 0xff;
	P3 = 0xff;

	EA  = 0;      //disable all interrupt 
	IT0 = 1;      //set INT0 type=falling edge 
	EX0 = 1;      //enable   INT0   
	EA = 1;       //enable all interrupt

	LongDelay(40);

	//LCD 초기화
	lcd_command_write(0x38);
	lcd_command_write(0x08);
	lcd_command_write(0x01);
	lcd_command_write(0x06);
	lcd_command_write(0x0C);
}

//LCD 문자열 출력 함수 선언
void LcdString(BYTE x, BYTE y, char *str) {
	BYTE index;
	BYTE character;

	lcd_command_write(0x80 + y + 0x40 * x);
	index = 0;
	character = str[index];

	while(character != '\0') {
		lcd_char_display(character);
		index++;
		character = str[index];
	}
}

//포트 KEY 값 읽어오는 함수 선언
BYTE GetPortKey(void) {
	BYTE value;
	value = P1;

	if (!(value & 0x80)) KeyNo = KEY7;
	else if (!(value & 0x40)) KeyNo = KEY6;
	else if (!(value & 0x20)) KeyNo = KEY5;
	else if (!(value & 0x10)) KeyNo = KEY4;
	else KeyNo = NOKEY;
	return KeyNo;
}

//세븐세그먼트 출력 함수 선언
void SegmentDisplay(WORD num) {
	WORD TempNumber;
	BYTE i;
	BYTE Digit = 0x08;   
	TempNumber = num;

	for(i=0; i < SegDigit; i++) {
		TempNumber = TempNumber / 10 ;
		Digit = Digit >> 1;
	}

	SEG_SIGAL = FND[TempNumber % 10];
	SEG_DIGIT =~Digit;

	if(SegDigit++ >= 3)
		SegDigit = 0;
}

//컴퓨터가 낼 가위바위보 난수 발생 함수 선언
int ChoiceOfCom(void) {
	int CountNumber=0;
	while(gate) {
		if((CountNumber++) > 999)
			CountNumber=0;
	}
	return (CountNumber%3+1);
}

//플레이어가 낼 가위바위보 함수 선언 
int ChoiceOfMe(void) {
	int MyChoiceNumber=0;
	BYTE gate2=1;
	while(gate2) {
		if(KeyNo == KEY7) { //포트 KEY 7번을 누르면 바위
			LcdString(1, 12, choice[0]);
			MyChoiceNumber=1; //바위의 숫자 반환값은 1
			gate2=0;
		}
		if (KeyNo == KEY6) { //포트 KEY 6번을 누르면 보자기
			LcdString(1, 12, choice[1]);
			MyChoiceNumber=2; //보자기의 숫자 반환값은 2
			gate2=0;
		}
		if (KeyNo == KEY5) { //포트 KEY 5번을 누르면 가위
			LcdString(1, 12, choice[2]);
			MyChoiceNumber=3; //가위의 숫자 반환값은 3
			gate2=0;
		}
	}
	return MyChoiceNumber; //가위바위보 숫자 반환
}

//플레이어 승수 계산 함수 선언
void IncreYouWinTimes(void) {
	if(FndData++ > 9999) 
		FndData = 0; 
}

//누가 이겼는지 LCD에 표시해주는 함수 선언
void WhoIsWinner(int com, int you) {
	int outcome = you - com; //플레이어의 가위바위보 반환값에서 컴퓨터의 가위바위보 반환값을 뺀 변수 선언

	switch(outcome) {
	case 0 : // 뺀 값이 0이면 무승부를 LCD에 출력
		LcdString(0, 7, "WE  ");
		LcdString(1, 7, "DRAW");
		return;
	case 1 : //뺀 값이 1 또는 -2이면 승리를 LCD에 출력하고 승수+1
	case -2 :
		LcdString(0, 7, "YOU ");
		LcdString(1, 7, "WIN ");
		IncreYouWinTimes();
		return;
	case 2 : //뺀 값이 2 또는 -1이면 패배를 LCD에 출력
	case -1 :
		LcdString(0, 7, "YOU ");
		LcdString(1, 7, "LOSE");
		return;
	default : //뺀 값이 그 외의 값이면 잠시 기다리도록 LCD에 출력
		LcdString(0, 7, "WAIT");
		LcdString(1, 7, "WAIT");
		return;
	}
}

//인터럽트가 발생하면 while 조건식의 변수가 0(거짓)이 되게 하여 while이 멈추게 한다.
ExternInterrupt0() interrupt  0  { //INT 0
	return gate=0;
}

void main(void) {
	int a = 1;
	Initial_CPU();
	LcdString(0, 0, "COM"); //LCD의 왼쪽에 COM 출력
	LcdString(0, 13,"ME");  //LCD의 오른쪽에 ME 출력

	while(1) {
		while(1){
			if(GetPortKey()==NOKEY) { //입력받은 포트 KEY 값이 없으면 포트 KEY 값을 한 번 더 입력받고 컴퓨터 가위바위보 난수 발생
				GetPortKey();
				com=ChoiceOfCom();
				continue;
			}
			else {
				com=ChoiceOfCom();	//컴퓨터의 가위바위보 반환 숫자값을 com 변수에 저장
				you=ChoiceOfMe(); //플레이어의 가위바위보 반환 숫자값을 you 변수에 저장

				if(com == 1) { //컴퓨터의 가위바위보 반환 값이 1이면 바위 LCD에 출력
					LcdString(1, 1, choice[0]);
				}
				if (com == 2) { //컴퓨터의 가위바위보 반환 값이 2이면 보자기 LCD에 출력
					LcdString(1, 1, choice[1]);
				}
				if (com == 3) { //컴퓨터의 가위바위보 반환 값이 3이면 가위 LCD에 출력
					LcdString(1, 1, choice[2]);
				}
			}
			WhoIsWinner(com,you);
			you=0;
			gate=1;

			SegmentDisplay(FndData); //세븐 세그먼트에 플레이어의 승수를 표시한다. 
			ShortDelay(100); //잔상을 없애기 위해
		}
	}
}
