/*  Program:        Lab04
**  Author:         Hayden Berry
**  Created Date:   02/17/23
**  Modified Date:  02/28/24
**
**  Description
**  -----------
**  This lab wires four buttons tied to different priority tasks.
**  We implement a round robin and a function queue method
**  for running these tasks.
**
**  Bugs: the shell is not "backspace-proof." 
**  Backspace works in many cases, but may cause undefined behavior
**  while typing an integer.
**  
**  Reference Links
**  ---------------
**  https://github.com/thomasfredericks/Bounce2
*/

#include <Arduino.h>
#include <esp.h>
#include <string.h>
#include <Bounce2.h> // to debounce pushbuttons

// forward declarations
void shell();
int parse(char *);
int task_time(char *);
void run_cmd(int);
void fq(); // function queue
void rr(); // round robin
void show();
void reset(int);
void help();
void welcome();
void set_devices(char *);
void set_service_time(struct device *, int);
void run_device(struct device*);
void complete_task(struct device *);
void button_check(struct device*);
bool occupied();
char typedKey();
void clearstr(char *,int);
int atoi(char);
int power(int,int); // x^y

// globals
#define dbinterval 40 // debounce interval

#define AButtonPin 21
#define BButtonPin 17
#define CButtonPin 16
#define DButtonPin 19

#define fqcmd    101
#define rrcmd    102
#define showcmd  103
#define resetcmd 104
#define helpcmd  105

#define round_robin 1
#define function_queue -1

int scheduler = round_robin; // default round robin
// buttons
Bounce2::Button red = Bounce2::Button();
Bounce2::Button yellow = Bounce2::Button();
Bounce2::Button blue = Bounce2::Button();
Bounce2::Button green = Bounce2::Button();


enum device_state {BLOCKED, READY, RUNNING};

struct device {
  char name;
  enum device_state state;
  int service_time;
  int response_time;
  int press_time;
  int start_time;
};

struct device a,b,c,d;

char input[50];
int i = 0;

void setup() {
  // Serial terminal for debug and interaction
  Serial.begin(115200);
  delay(2000);
  Serial.print("\nSerial ready!\n");

  // button initilization
  red.attach(AButtonPin, INPUT_PULLUP);
  red.interval(dbinterval);
  red.setPressedState(LOW);

  yellow.attach(BButtonPin, INPUT_PULLUP);
  yellow.interval(dbinterval);
  yellow.setPressedState(LOW);

  green.attach(CButtonPin, INPUT_PULLUP);
  green.interval(dbinterval);
  green.setPressedState(LOW);

  blue.attach(DButtonPin, INPUT_PULLUP);
  blue.interval(dbinterval); 
  blue.setPressedState(LOW);

  a.state = BLOCKED;
  b.state = BLOCKED;
  c.state = BLOCKED;
  d.state = BLOCKED;

  a.name = 'A';
  b.name = 'B';
  c.name = 'C';
  d.name = 'D';

  a.response_time = 0;
  b.response_time = 0;
  c.response_time = 0;
  d.response_time = 0;

  a.press_time = 0;
  b.press_time = 0;
  c.press_time = 0;
  d.press_time = 0;

  reset(0); // initialize service times

  welcome();
  Serial.print(">");
}

void loop() {
  // button checks
  button_check(&a);
  button_check(&b);
  button_check(&c);
  button_check(&d);
  shell(); 
  delay(5);
}

// serial interface
void shell() {
  char key = 0;
  while (key != '\n' && key != '\r') {
    key = typedKey();
    if (key == '\n') {
      Serial.print(">");
    }
    if (i > 49 || key == 0) {
      return;
    }
    if (key != 0 && key != '\n' && key != '\r' && key != '\b') {
      input[i] = key;
      i++;
    }
    if (key == '\b') {
      i--;
    }
  }
  run_cmd(parse(input));

  clearstr(input,50);
  i = 0;
  return;
}

int parse(char * input) {
  int cmd_code = 0;
  if (strstr(input,"fq") != 0) {
    cmd_code = fqcmd;
  }
  else if (strstr(input,"rr") != 0) {
    cmd_code = rrcmd;
  }
  else if (strstr(input,"show") != 0) {
    cmd_code = showcmd;
  }
  else if (strstr(input,"reset") != 0) {
    cmd_code = resetcmd;
  }
  else if (strstr(input,"help") != 0) {
    cmd_code = helpcmd;
  }
  else {
    cmd_code = 0; // do nothing in run_cmd
    set_devices(input);
  }
  return cmd_code;
}

int task_time(char * input) {
  // backspacing while typing integers DOES NOT WORK
  int time = 0;
  int length = 0;
  int i = 0;
  while (input[i] >= '0' && input[i] <= '9') {
    length++;
    i++;
  }
  i = 0;
  while (input[i] >= '0' && input[i] <= '9') {
      time += atoi(input[i])*power(10,length-i-1); // time += atoi(input[i]) * 10^(length-i-1)
      i++;
  }
  return time;
}

void run_cmd(int cmd) {
  switch(cmd) {
    case(fqcmd):
      Serial.println();
      fq();
      break;
    case(rrcmd):
      Serial.println();
      rr();
      break;
    case(showcmd):
      Serial.println();
      show();
      break;
    case(resetcmd):
      Serial.println();
      reset(1);
      break;
    case(helpcmd):
      Serial.println();
      help();
      break;
    default:
      Serial.print("error: command not found\n");
      break;
  }
  return;
}

// command functions
void fq() {
  if (scheduler == round_robin) {
    scheduler *= -1;
  }
  // scheduler set to fq
  Serial.print("Change to function queue.");
  return;
}

void rr() {
  if (scheduler == function_queue) {
    scheduler *= -1;
  }
  // scheduler set to rr
  Serial.print("Change to round robin.");
  return;
}

void show() {
  Serial.println();
  Serial.print("Scheduler: ");
  if (scheduler == 1) {
    Serial.println("Round Robin");
  }
  if (scheduler == -1) {
    Serial.println("Function Queue Scheduling");
  }
  Serial.println("Service Times:");
  Serial.print("  DeviceA: ");
  Serial.println(a.service_time);
  Serial.print("  DeviceB: ");
  Serial.println(b.service_time); //
  Serial.print("  DeviceC: ");
  Serial.println(c.service_time); //
  Serial.print("  DeviceD: ");
  Serial.println(d.service_time); //
  return;
}

void reset(int i) { 
  // initialize and, if i, print the show menu
  set_service_time(&a,0);
  set_service_time(&b,0);
  set_service_time(&c,0);
  set_service_time(&d,0);
  if (i) {
    Serial.print("Reset all service times to zero.\n");
    show();
  }
}

void help() {
  Serial.println();
  Serial.println("Scheduler Simulator Commands");
  Serial.println("============================");
  Serial.println();
  Serial.println("   fq - Use Function Queue Scheduling");
  Serial.println("   rr - Use Round Robin Queue Scheduling");
  Serial.println(" show - Show service times for all devices");
  Serial.println("reset - Reset all device service times to zero");
  Serial.println(" help - Show this help message");
  Serial.println();
  Serial.println("To set device service times, enter the device letter");
  Serial.println("(a-d) followed by a space and the service time in mS.");
  Serial.println("You may enter more than one device and service time by");
  Serial.println("separating names and times with spaces. For example:");
  Serial.println();
  Serial.println("     a 3200 b 2500 c 1250 d 1234");
  Serial.println();
  Serial.println("To request service time for a device, press its button.");
  Serial.println("You may only request service for blocked devices (i.e.");
  Serial.println("not in the Ready or Running state)");
  return;
}

// welcome message
void welcome() {
  Serial.println();
  Serial.println("RR/FQS Scheduling Simulator");
  Serial.println("===========================");
  Serial.println("   (type help for help)    ");
  return;
}

// tasks
void set_devices(char* input) {
  char* token = strtok(input," ");
  char* letter = 0;
  int time = 0;
  while (token != nullptr) {
    // letter
    letter = token;
    token = strtok(nullptr," ");
    // time
    if (token != nullptr) {
      time = task_time(token);
      token = strtok(nullptr," ");
    }
    if (!strcmp(letter,"a")) {
      Serial.println();
      set_service_time(&a,time);
      Serial.print("Device: a  ServiceTime: ");
      Serial.print(time);
    }
    else if (!strcmp(letter,"b")) {
      Serial.println();
      set_service_time(&b,time);
      Serial.print("Device: b  ServiceTime: ");
      Serial.print(time);
    }
    else if (!strcmp(letter,"c")) {
      Serial.println();
      set_service_time(&c,time);
      Serial.print("Device: c  ServiceTime: ");
      Serial.print(time);
    }
    else if (!strcmp(letter,"d")) {
      Serial.println();
      set_service_time(&d,time);
      Serial.print("Device: d  ServiceTime: ");
      Serial.print(time);
    }
    else {
      Serial.println();
      Serial.print("ERROR: invalid cmd line entry.  Try again!");
      return;
    }
  }
  return;
}

void set_service_time(struct device *letter, int time) {
  letter->service_time = time;
  return;
}

void run_device(struct device *letter) {
  letter->state = RUNNING;
  Serial.print("Device ");
  Serial.print(letter->name);
  Serial.println(" Running.");
  letter->response_time = letter->start_time - letter->press_time + letter->service_time;
  return;
}

void complete_task(struct device *letter) {
  letter->state = BLOCKED;
  Serial.print("Device ");
  Serial.print(letter->name);
  Serial.println(" Stopped.");

  Serial.print("Device ");
  Serial.print(letter->name);
  Serial.print(" response time: ");
  Serial.println(letter->response_time);
  letter->response_time = 0;
  return;
}

void button_check(struct device * letter) { // this function does all of the button nonsense
  int current_time = millis();
  bool button_pressed;
  switch(letter->name) {
    case('A'):
      red.update();
      button_pressed = red.pressed();
      break;
    case('B'):
      yellow.update();
      button_pressed = yellow.pressed();
      break;
    case('C'):
      green.update();
      button_pressed = green.pressed();
      break;
    case('D'):
      blue.update();
      button_pressed = blue.pressed();
      break;
  }
  if (button_pressed) {
    // READY DEVICE
    if (letter->state == RUNNING) { // we should not be able to run the same task twice
      Serial.print("Device ");
      Serial.print(letter->name);
      Serial.println(" request ignored (Task not Stopped!)");
      return;
    }
    if (letter->state == BLOCKED) {
      if (letter->service_time == 0) {
        Serial.print("Device ");
        Serial.print(letter->name);
        Serial.println(" request ignored (service time = 0)");
      }
      else {
        letter->state = READY;
        Serial.print("Device " );
        Serial.print(letter->name);
        Serial.println(" Ready");
        letter->press_time = current_time;
      }
    }
  }
  if (!occupied() && letter->state == READY) { // if no other devices are running...
    // we gotta run something!
    bool update = 0; // are we going to run this task or not?
    if (scheduler == round_robin) {
      // round robin means run tasks in the order pressed (earliest press_time).
      // blocked tasks are irrelevant, 
      if ((letter->press_time <= a.press_time || a.state == BLOCKED) 
      && 
      (letter->press_time <= b.press_time || b.state == BLOCKED) 
      &&
      (letter->press_time <= c.press_time || c.state == BLOCKED) 
      && 
      (letter->press_time <= d.press_time || d.state == BLOCKED)) {
        update = 1;
      }
    }
    else if (scheduler == function_queue) {
      // function queue means run tasks by priority
      if (
        (letter->name == 'A')
        || 
        (letter->name == 'B' && a.state == BLOCKED)
        || 
        (letter->name == 'C' && a.state == BLOCKED && b.state == BLOCKED)
        || 
        (letter->name == 'D' && a.state == BLOCKED && b.state == BLOCKED && c.state == BLOCKED)) {
        update = 1;
      }
    }
    if (update) {
      letter->start_time = current_time;
      run_device(letter); // ...start running the device
    }
  }
  if (letter->state == RUNNING && ((current_time - letter->start_time > letter->service_time) || letter->service_time == 0)) {
    // STOP DEVICE
    // when the time passed becomes > service time, it means it's done 
    // when the service time IS 0, it is also done
    complete_task(letter);
    letter->press_time = 0;
    letter->start_time = 0;
  }
  return;
}

bool occupied() { // checks if the cpu is occupied (if any tasks are being run)
  return (a.state == RUNNING || b.state == RUNNING || c.state == RUNNING || d.state == RUNNING);
}

char typedKey() {
  char key;
  if(Serial.available() > 0) {
	  key = Serial.read();
    if (key > 64 && key < 97) {
      key += 32; // all UPPER CASE letters are converted to lower case
    }
    if (key == '\b') {
      Serial.print("\b ");
    }
    Serial.print(key);
    return key;
	}
  return 0;
}

void clearstr(char * array, int length) {
  for(int i = 0; i<length; i++) {
    array[i] = 0;
  }
  return;
}

int atoi(char c) { // converts a time character to an integer
  return((int)c - 48);
}

int power(int b, int n) { // x = b^n

  int x = 1;
  for(int i = 0; i<n; i++) {
    x *= b;
  }
  return x;
}