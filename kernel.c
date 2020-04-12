#include "kernel.h"
#include "keyboard.h"
/*
16 bit video buffer elements(register ax)
8 bits(ah) higher : 
  lower 4 bits - forec olor
  higher 4 bits - back color

8 bits(al) lower :
  8 bits : ASCII character to print
*/
uint32 index=0;
uint32 line=1;
uint32 flag1=0, flag2=0;
char cmd[cmd_size];
char file1[720];
uint32 cmd_index=0, create_req=0, create_count=0, f_created=0, open_req=0, open_count=0, is_open=0, file_counter=5;
char get_ascii_char(uint8 key_code)
{
  switch(key_code){
    case KEY_A : return 'A';
    case KEY_B : return 'B';
    case KEY_C : return 'C';
    case KEY_D : return 'D';
    case KEY_E : return 'E';
    case KEY_F : return 'F';
    case KEY_G : return 'G';
    case KEY_H : return 'H';
    case KEY_I : return 'I';
    case KEY_J : return 'J';
    case KEY_K : return 'K';
    case KEY_L : return 'L';
    case KEY_M : return 'M';
    case KEY_N : return 'N';
    case KEY_O : return 'O';
    case KEY_P : return 'P';
    case KEY_Q : return 'Q';
    case KEY_R : return 'R';
    case KEY_S : return 'S';
    case KEY_T : return 'T';
    case KEY_U : return 'U';
    case KEY_V : return 'V';
    case KEY_W : return 'W';
    case KEY_X : return 'X';
    case KEY_Y : return 'Y';
    case KEY_Z : return 'Z';
    case KEY_1 : return '1';
    case KEY_2 : return '2';
    case KEY_3 : return '3';
    case KEY_4 : return '4';
    case KEY_5 : return '5';
    case KEY_6 : return '6';
    case KEY_7 : return '7';
    case KEY_8 : return '8';
    case KEY_9 : return '9';
    case KEY_0 : return '0';
    case KEY_MINUS : return '-';
    case KEY_EQUAL : return '=';
    case KEY_SQUARE_OPEN_BRACKET : return '[';
    case KEY_SQUARE_CLOSE_BRACKET : return ']';
    case KEY_SEMICOLON : return ';';
    case KEY_BACKSLASH : return '\\';
    case KEY_COMMA : return ',';
    case KEY_DOT : return '.';
    case KEY_FORESLHASH : return '/';
    case KEY_SPACE : return ' ';
    default : return 0;
  }
}	

uint16 vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color) 
{
  uint16 ax = 0;
  uint8 ah = 0, al = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

//clear video buffer array
void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
  uint32 i;
  for(i = 0; i < BUFSIZE; i++){
    (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
  }
}

//initialize vga buffer
void init_vga(uint8 fore_color, uint8 back_color)
{
  vga_buffer = (uint16*)VGA_ADDRESS;  //point vga_buffer pointer to VGA_ADDRESS 
  clear_vga_buffer(&vga_buffer, fore_color, back_color);  //clear buffer
}

void print_str(char *str){
	uint32 i=0;
	while(str[i]){
		vga_buffer[index] = vga_entry(str[i], WHITE, BLACK);
		i++;
		index++;
	}
}

void print_newline(){
	if(line>55){
		line=0;
		clear_vga_buffer(&vga_buffer, WHITE, BLACK);
	}
	index = 80*line;
	line++; 
}

uint32 total_digit(uint32 num){

  uint32 count = 0;
  if(num == 0)
    return 1;
  while(num > 0){
    count++;
    num = num/10;
  }
  return count;
}
void print_num(uint32 num){
	uint32 count = total_digit(num);
	char str[count+1];
	uint32 i=0;
	while(num){
		uint8 ch = num % 10;
		str[i] = ch+'0';
		num /= 10;
		i++;
	}
	print_str(str);
}

uint8 inb(uint16 port)
{
  uint8 ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}


void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}


char get_input_keycode()
{
  char ch = 0;
  while((ch = inb(KEYBOARD_PORT)) != 0){
    if(ch > 0)
      return ch;
  }
  return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32 timer_count)
{
  wait_for_io(timer_count);
}

void print_char(char ch){
	vga_buffer[index] = vga_entry(ch, WHITE, BLACK);
	index++;
}
void test_input()
{
  char ch = 0;
  char keycode = 0;
  do{
    keycode = get_input_keycode();
    if(keycode == KEY_ENTER){
      print_newline();
    }else{
      ch = get_ascii_char(keycode);
      print_char(ch);
      cmd[cmd_index] = ch;
      
      // increment character count
      if(create_req==1 && create_count<5){
      	create_count++;
      }
      
      // name complete
      if(create_count==5){
      	create_req=0;
      	create_count=0;
      	uint32 i=0;
      	while(i<5){
      		file1[i] = cmd[cmd_index-4+i];
      		i++;
      	}
      	print_newline();
      	print_str("file sucessfully created");
      	print_newline();
      	i=0;
      	while(i<5){
      		print_char(file1[i]);
      		i++;
      	}
      	print_newline();
      	print_str("enter OPEN-F to open files for writing");
      	print_newline();
      	f_created++;
      	print_newline();
      }
      
      if(open_req==1 && open_count<5){
      	open_count++;
      }
      
      if(is_open==1){
      		file1[file_counter]=ch;
      		file_counter++;
      }
      
      if(open_count==5){
      	open_req=0;
      	open_count=0;
      	uint32 i=0;
      	uint32 check=1;
      	while(i<5){
      		if(cmd[cmd_index-4+i]!=file1[i]){
      			check=0;
      			break;
      		}
      		i++;
      	}
      	if(check==1){
      		print_newline();
      		print_str("file sucessfully opened for writing");
      		print_newline();
      		i=5;
      		while(i<file_counter){
      			print_char(file1[i]);
      			i++;
      		}
      		
      		print_newline();
      		is_open=1;
      	}
      	else{
      		print_newline();
      		print_str("wrong file name");
      		print_newline();      		
      	}
      }
      // condition checking
      if(cmd_index>8){
      	if(cmd[cmd_index]=='E' && cmd[cmd_index-1]=='L' && cmd[cmd_index-2]=='I' && cmd[cmd_index-3]=='F' && cmd[cmd_index-4]=='-' && cmd[cmd_index-5]=='E' && cmd[cmd_index-6]=='T' && cmd[cmd_index-7]=='A' && cmd[cmd_index-8]=='E' && cmd[cmd_index-9]=='R' && cmd[cmd_index-10]=='C' ){
      		print_newline();
      		print_str("Please enter file name");
      		print_newline();
      		create_req = 1;
      		create_count=0;
      	}
      }
      
      if(cmd_index>5 && f_created>0){
      	if(cmd[cmd_index]=='F' && cmd[cmd_index-1]=='-' && cmd[cmd_index-2]=='N' && cmd[cmd_index-3]=='E' && cmd[cmd_index-4]=='P' && cmd[cmd_index-5]=='O'){
      		print_newline();
      		print_str("please enter file to be opened");
      		print_newline();
      		open_req=1;
      		open_count=0;
      	}
      }
    }
    
    if(is_open==1 && cmd_index>3){
    	if(cmd[cmd_index]== 'E' && cmd[cmd_index-1]== 'V' && cmd[cmd_index-2]== 'A' && cmd[cmd_index-3]== 'S'){
    		is_open=0;
    		print_newline();
    		print_str("file saved sucessfullly");
    		print_newline();
    	}
    }
    
    cmd_index = (cmd_index + 1)%cmd_size;
    sleep(0x06FFFFFF);
  }while(ch > 0);
}

void kernel_entry()
{
  //first init vga with fore & back colors
  init_vga(WHITE, BLACK);
  print_str("Hello OS_EDI is up and running, its starting point 0xb8000");
  /*print_newline();
  print_str("New line is working");
  print_newline();
  print_str("SHUBHAMS OPERATING SYSTEM");
  print_newline();
  print_str("printing done");
  print_newline();
  print_str("dynamic length str print");
  print_newline();
  print_str("newline print done by eexploiting that each line has 80 indices");
  print_newline();
  print_num(3678882);
  */
  print_newline();
  test_input();
}
