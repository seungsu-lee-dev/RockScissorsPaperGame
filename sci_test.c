#include "global.h"

#define SEGMENT_TIME 500

//���켼�׸�Ʈ ���� ���� ����, �ʱ�ȭ
BYTE SegDigit = 0;
WORD FndData = 0;

//����, ����, �� �迭
int *choice[]={"ROCK","PAP ","SCI "};

//��ǻ�Ϳ� ���� ���������� ��ȯ ���� ������ ���� ����, �ʱ�ȭ
static int com = 0;
static int you = 0;

//�Է��� ��Ʈ KEY ����
#define NOKEY 0
#define KEY7 7
#define KEY6 6
#define KEY5 5
#define KEY4 4

//while�� ���ǽ� ���� ����, �ʱ�ȭ
BYTE gate=1;

//��Ʈ KEY�� ������ ���� ����, �ʱ�ȭ
BYTE KeyNo = NOKEY;

//���켼�׸�Ʈ�� ���� ǥ��
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

//������ �Լ� ����
void ShortDelay(BYTE i) {
	while(i--);    // 7us delay   about 7 Cycle
}
void LongDelay(WORD i) {
	while(i--) {
		ShortDelay(145);  // 1ms delay 
	}
}

//LCD ȭ�� Ŀ�� ��ġ, ���� ǥ�� �Լ� ����
void lcd_command_write(BYTE command) {
	LCD_WR_COMMAND = command;
	LongDelay(1);
}
void lcd_char_display(BYTE character) {
	LCD_WR_DATA = character;
	LongDelay(1);
}

//��Ʈ, ���ͷ�Ʈ ����
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

	//LCD �ʱ�ȭ
	lcd_command_write(0x38);
	lcd_command_write(0x08);
	lcd_command_write(0x01);
	lcd_command_write(0x06);
	lcd_command_write(0x0C);
}

//LCD ���ڿ� ��� �Լ� ����
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

//��Ʈ KEY �� �о���� �Լ� ����
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

//���켼�׸�Ʈ ��� �Լ� ����
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

//��ǻ�Ͱ� �� ���������� ���� �߻� �Լ� ����
int ChoiceOfCom(void) {
	int CountNumber=0;
	while(gate) {
		if((CountNumber++) > 999)
			CountNumber=0;
	}
	return (CountNumber%3+1);
}

//�÷��̾ �� ���������� �Լ� ���� 
int ChoiceOfMe(void) {
	int MyChoiceNumber=0;
	BYTE gate2=1;
	while(gate2) {
		if(KeyNo == KEY7) { //��Ʈ KEY 7���� ������ ����
			LcdString(1, 12, choice[0]);
			MyChoiceNumber=1; //������ ���� ��ȯ���� 1
			gate2=0;
		}
		if (KeyNo == KEY6) { //��Ʈ KEY 6���� ������ ���ڱ�
			LcdString(1, 12, choice[1]);
			MyChoiceNumber=2; //���ڱ��� ���� ��ȯ���� 2
			gate2=0;
		}
		if (KeyNo == KEY5) { //��Ʈ KEY 5���� ������ ����
			LcdString(1, 12, choice[2]);
			MyChoiceNumber=3; //������ ���� ��ȯ���� 3
			gate2=0;
		}
	}
	return MyChoiceNumber; //���������� ���� ��ȯ
}

//�÷��̾� �¼� ��� �Լ� ����
void IncreYouWinTimes(void) {
	if(FndData++ > 9999) 
		FndData = 0; 
}

//���� �̰���� LCD�� ǥ�����ִ� �Լ� ����
void WhoIsWinner(int com, int you) {
	int outcome = you - com; //�÷��̾��� ���������� ��ȯ������ ��ǻ���� ���������� ��ȯ���� �� ���� ����

	switch(outcome) {
	case 0 : // �� ���� 0�̸� ���ºθ� LCD�� ���
		LcdString(0, 7, "WE  ");
		LcdString(1, 7, "DRAW");
		return;
	case 1 : //�� ���� 1 �Ǵ� -2�̸� �¸��� LCD�� ����ϰ� �¼�+1
	case -2 :
		LcdString(0, 7, "YOU ");
		LcdString(1, 7, "WIN ");
		IncreYouWinTimes();
		return;
	case 2 : //�� ���� 2 �Ǵ� -1�̸� �й踦 LCD�� ���
	case -1 :
		LcdString(0, 7, "YOU ");
		LcdString(1, 7, "LOSE");
		return;
	default : //�� ���� �� ���� ���̸� ��� ��ٸ����� LCD�� ���
		LcdString(0, 7, "WAIT");
		LcdString(1, 7, "WAIT");
		return;
	}
}

//���ͷ�Ʈ�� �߻��ϸ� while ���ǽ��� ������ 0(����)�� �ǰ� �Ͽ� while�� ���߰� �Ѵ�.
ExternInterrupt0() interrupt  0  { //INT 0
	return gate=0;
}

void main(void) {
	int a = 1;
	Initial_CPU();
	LcdString(0, 0, "COM"); //LCD�� ���ʿ� COM ���
	LcdString(0, 13,"ME");  //LCD�� �����ʿ� ME ���

	while(1) {
		while(1){
			if(GetPortKey()==NOKEY) { //�Է¹��� ��Ʈ KEY ���� ������ ��Ʈ KEY ���� �� �� �� �Է¹ް� ��ǻ�� ���������� ���� �߻�
				GetPortKey();
				com=ChoiceOfCom();
				continue;
			}
			else {
				com=ChoiceOfCom();	//��ǻ���� ���������� ��ȯ ���ڰ��� com ������ ����
				you=ChoiceOfMe(); //�÷��̾��� ���������� ��ȯ ���ڰ��� you ������ ����

				if(com == 1) { //��ǻ���� ���������� ��ȯ ���� 1�̸� ���� LCD�� ���
					LcdString(1, 1, choice[0]);
				}
				if (com == 2) { //��ǻ���� ���������� ��ȯ ���� 2�̸� ���ڱ� LCD�� ���
					LcdString(1, 1, choice[1]);
				}
				if (com == 3) { //��ǻ���� ���������� ��ȯ ���� 3�̸� ���� LCD�� ���
					LcdString(1, 1, choice[2]);
				}
			}
			WhoIsWinner(com,you);
			you=0;
			gate=1;

			SegmentDisplay(FndData); //���� ���׸�Ʈ�� �÷��̾��� �¼��� ǥ���Ѵ�. 
			ShortDelay(100); //�ܻ��� ���ֱ� ����
		}
	}
}
