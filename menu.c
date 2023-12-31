#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <linux/input.h>

#define MENU_ITEMS 3
#define MENU_ITEM_HEIGHT 40
#define MENU_ITEM_SPACING 10

int log_fd;
int gpio_keys_polled_fd;
int event_0_fd;
pthread_t adc_thread;

#define RG35XX_KEY_UP 0x1
#define RG35XX_KEY_DOWN 0x2
#define RG35XX_KEY_LEFT 0x4
#define RG35XX_KEY_RIGHT 0x8

#define RG35XX_KEY_A 0x10
#define RG35XX_KEY_B 0x20
#define RG35XX_KEY_Y 0x40
#define RG35XX_KEY_X 0x80

#define RG35XX_KEY_VOL_UP 0x4000
#define RG35XX_KEY_VOL_DOWN 0x8000

#define RG35XX_KEY_L1 0x400
#define RG35XX_KEY_R1 0x800
#define RG35XX_KEY_L2 0x1000
#define RG35XX_KEY_R2 0x2000

#define RG35XX_KEY_START 0x100
#define RG35XX_KEY_SELECT 0x200

#define RG35XX_KEY_MENU 0x10000

void open_gpio_keys_polled()
{
  DIR *dir;           // r4
  const char *d_name; // r7
  int i;              // r5
  int fd;             // r0
  int tmp;            // r6
  struct dirent *d;   // r0
  char v6[128];       // [sp+0h] [bp-298h] BYREF
  char s[536];        // [sp+80h] [bp-218h] BYREF

  dir = opendir("/dev/input");
  if (dir)
  {
    i = 7;
    while (1)
    {
      d = readdir(dir);
      if (!d)
        break;
      d_name = d->d_name;
      if (!strncmp(d->d_name, "event", 5u))
      {
        if (!--i)
          break;
        sprintf(s, "/dev/input/%s", d_name);
        fd = open(s, 0);
        tmp = fd;
        if (fd > 0 && ioctl(fd, EVIOCGNAME(128)/*0x80804506*/, v6) >= 0)
        {
          if (!strcmp(v6, "gpio-keys-polled"))
          {
            gpio_keys_polled_fd = tmp;
            break;
          }
          close(tmp);
        }
      }
    }
    closedir(dir);
  }
}

uint32_t adc_table[17] = {
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61,
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x6C, 0x6D};

struct input_event_t
{
  uint64_t time;
  uint16_t type;
  uint16_t code;
  uint32_t value;
};

int keep_going = 1;
int input_var_xx = 0;
int event_code = 0;
int event_value = 0;
int key_state;
int input_var_9 = 0;
int input_var_10 = 0;
int input_var_17 = 0;


void process_events(int ev_code, int ev_value)

{
  int tmp;

  if (adc_table[0] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 1;
    }
    else
    {
      key_state = key_state & 0xfffffe;
    }
  }
  if (adc_table[1] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 2;
    }
    else
    {
      key_state = key_state & 0xfffffd;
    }
  }
  if (adc_table[2] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 4;
    }
    else
    {
      key_state = key_state & 0xfffffb;
    }
  }
  if (adc_table[3] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 8;
    }
    else
    {
      key_state = key_state & 0xfffff7;
    }
  }
  if ((key_state & 0xc) == 0xc)
  {
    if (adc_table[2] == ev_code)
    {
      key_state = key_state & 0xfffff7;
    }
    else
    {
      key_state = key_state & 0xfffffb;
    }
  }
  if ((key_state & 3) == 3)
  {
    if (adc_table[0] == ev_code)
    {
      key_state = key_state & 0xfffffd;
    }
    else
    {
      key_state = key_state & 0xfffffe;
    }
  }
  if (adc_table[4] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x10;
    }
    else
    {
      key_state = key_state & 0xffffef;
    }
  }
  if (adc_table[5] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x20;
    }
    else
    {
      key_state = key_state & 0xffffdf;
    }
  }
  if (adc_table[6] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x80;
    }
    else
    {
      key_state = key_state & 0xffff7f;
    }
  }
  if (adc_table[7] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x40;
    }
    else
    {
      key_state = key_state & 0xffffbf;
    }
  }
  if (adc_table[9] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x200;
    }
    else
    {
      key_state = key_state & 0xfffdff;
    }
  }
  if (adc_table[8] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x100;
    }
    else
    {
      key_state = key_state & 0xfffeff;
    }
  }
  if (adc_table[10] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x400;
    }
    else
    {
      key_state = key_state & 0xfffbff;
    }
  }
  if (adc_table[11] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x800;
    }
    else
    {
      key_state = key_state & 0xfff7ff;
    }
  }
  if (adc_table[12] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x1000;
    }
    else
    {
      key_state = key_state & 0xffefff;
    }
  }
  if (adc_table[13] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x2000;
    }
    else
    {
      key_state = key_state & 0xffdfff;
    }
  }
  if (adc_table[14] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x10000;
    }
    else
    {
      key_state = key_state & 0xfeffff;
    }
  }
  if (adc_table[15] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x4000;
      input_var_10 = ev_value;
    }
    else
    {
      input_var_10 = 0;
      key_state = key_state & 0xffbfff;
    }
  }
  if (adc_table[16] == ev_code)
  {
    if (ev_value == 1)
    {
      key_state = key_state | 0x8000;
      input_var_10 = ev_value;
    }
    else
    {
      input_var_10 = 0;
      key_state = key_state & 0xff7fff;
    }
  }

  //TODO: this is for shortcut handling
  if ((key_state & KEY_MENU) == 0)
  {
    tmp = 0;
    if (input_var_9 == 2)
    {
      if (input_var_17 == 0)
      {
        tmp = 1;
      }
    }
    else if (input_var_9 != 3)
    {
      return;
    }
  }
  else
  {
    if (input_var_9 != 3)
    {
      input_var_9 = 2;
    }
    if (input_var_10 != 1)
    {
      return;
    }
    tmp = 3;
  }
  input_var_9 = tmp;

  return;
}

void *read_adc2key_thread(void *dummy)
{
  struct input_event_t input_ev;
  struct pollfd polldata[2];
  ssize_t rd;
  int fd;
  int ret;

  while (1)
  {
    if (!keep_going)
    {
      return dummy;
    }
    memset(polldata, 0, 0x10);
    if (0 < gpio_keys_polled_fd)
    {
      polldata[0].fd = gpio_keys_polled_fd;
      polldata[0].events = POLLIN;
    }
    if (event_0_fd < 1)
    {
      if (0 >= gpio_keys_polled_fd)
      {
        printf("--<%s>-- not find event\n", "read_adc2key_thread");
        return dummy;
      }
    }
    else
    {
      polldata[1].fd = event_0_fd;
      polldata[1].events = POLLIN;
    }
    ret = poll(polldata, 2, 300);
    if (ret < 0)
    {
      printf("<%s> ctrl+c is pressed ret=%d\n", "read_adc2key_thread");
      keep_going = 0;
      input_var_xx = 0x100000;
      return dummy;
    }
    for (int i = 0; i < 2; i++)
    {
      if ((polldata[i].revents & POLLIN) != 0)
      {
        if (i == 0)
        {
          fd = gpio_keys_polled_fd;
          rd = read(polldata[0].fd, &input_ev, 0x10);
          if (rd < 0)
          {
            printf("read keypad error ");
          }
          else
          {
            if (input_ev.type != 1)
              continue;
            event_code = input_ev.code;
            event_value = input_ev.value;
            process_events(event_code, input_ev.value);
          }
        }
        else
        {
          fd = event_0_fd;
          rd = read(polldata[1].fd, &input_ev, 0x10);
          if (rd < 0)
          {
            printf("read powerkey error ");
          }
          else
          {
            // handle various power shortcuts here
            //  if (input_ev.type == 1) {
            //    if (input_ev.value == 0) {
            //      remove_timer();
            //      if (DAT_0002b650 != 0) {
            //        cpu_and_brightness(1);
            //      }
            //    }
            //    else {
            //      if (input_ev.value != 1) goto LAB_0000b7ea;
            //      add_timer();
            //    }
            //    set_sdl_tick();
            //  }
            // }
          }
        }
      }
    }
  }
}

int open_adc_bnt_input()
{
  open_gpio_keys_polled();
  if (gpio_keys_polled_fd <= 0)
  {
    printf("Open %s error\n", "/dev/input/event1");
    return -1;
  }
  event_0_fd = open("/dev/input/event0", 0);
  if (event_0_fd < 0)
  {
    printf("Open %s error\n", "/dev/input/event0");
    close(gpio_keys_polled_fd);
    return -1;
  }
  if (pthread_create(&adc_thread, 0, read_adc2key_thread, 0))
    return -1;
  else
    return 0;
}

int main(int argc, char *argv[])
{
  int fd = open("/mnt/mmc/CFW/os/log.txt", O_WRONLY | O_CREAT | O_TRUNC);
  log_fd = fd;

  dup2(fd, 1); // redirect stdout to log file
  dup2(fd, 2); // redirect stderr to log file

  printf("starting up\n");
  setenv("SDL_NOMOUSE", "1", 1);
  if (SDL_Init(33) < 0)
  {

    const char *Error = SDL_GetError();
    printf("SDL_Init error %s\n", Error);
    return 0;
  }
  if (TTF_Init())
  {
    printf("Unable to initialize SDL_ttf: %s\n", TTF_GetError());
    return 1;
  }
  
  SDL_ShowCursor(0);

  int r = SDL_VideoModeOK(640, 480, 16, 0);
  // if not OK, then exit
  if (r == 0)
  {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, 0);
  if (screen == NULL)
  {
    printf("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
  SDL_Flip(screen);
  SDL_EventState(1, 0);
  SDL_EventState(2, 0);
  SDL_EventState(3, 0);

  TTF_Font *font = TTF_OpenFont("/mnt/vendor/bin/default.ttf", 22); // Specify the path to a TrueType font
  if (font == NULL)
  {
    printf("Unable to load font: %s\n", TTF_GetError());
    return 1;
  }

  SDL_Color color = {10, 10, 10};
  SDL_Surface *text_start_adb = TTF_RenderText_Blended(font, "SELECT -> Start ADB", color);
  SDL_Surface *text_start_menu = TTF_RenderText_Blended(font, "START -> Start Default Menu", color);
  SDL_Rect rect1 = {10, 10, 0, 0};
  SDL_BlitSurface(text_start_adb, NULL, screen, &rect1);
  SDL_Rect rect2 = {10, 50, 0, 0};
  SDL_BlitSurface(text_start_menu, NULL, screen, &rect2);
  

  int res = open_adc_bnt_input();
  fflush(stdout);
  sync(); //flush log

  SDL_Rect rect3 = {10, 100, 200, 50};
  while (1)
  {
    char buf[100];
    snprintf(buf, 100, "key_state: %08x", key_state);
    // draw text
    SDL_FillRect(screen, &rect3, SDL_MapRGB(screen->format, 255, 255, 255));
    
    SDL_Surface *text = TTF_RenderText_Blended(font, buf, color);
    SDL_Rect rect = {10, 100, 0, 0};
    SDL_BlitSurface(text, NULL, screen, &rect);
    SDL_Flip(screen);

    if (key_state == RG35XX_KEY_SELECT)
    {
      //exec script
      printf("starting script\n");
      pid_t pid = fork();
      if (pid == 0)
      {
        //child
        execl("/system/bin/sh", "sh", "-c", "/system/appres/bin/ADB_ON.sh", NULL);        
        exit(0);
      }
      else
      {
        //parent
        printf("waiting for script to exit\n");
        sync();
        waitpid(pid, NULL, 0);
        SDL_Color color = {10, 255, 10};
        SDL_Surface *started = TTF_RenderText_Blended(font, "ADB Started", color);
        SDL_Rect rect1 = {500, 10, 0, 0};
        SDL_BlitSurface(started, NULL, screen, &rect1);
      }
    }
    if (key_state == RG35XX_KEY_START)
    {
      break;
    }    
    SDL_FreeSurface(text);
    SDL_Delay(100);
  }

  SDL_FreeSurface(screen);

  TTF_CloseFont(font);
  TTF_Quit();

  SDL_Quit();

  //start orig.bin
  printf("starting orig.bin\n");
  keep_going = 0;
  //fork
  pid_t pid = fork();
  if (pid == 0)
  {
    //child
    char *args[] = {"/mnt/vendor/bin/orig.bin", NULL};
    execv(args[0], args);
  }
  else
  {
    //parent
    printf("waiting for orig.bin to exit\n");
    waitpid(pid, NULL, 0);
  }

  printf("done\n");

  return 0;
}
