/* MotorControll.cpp */
#include <vector>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdio>
#include <ros/ros.h>$

#define MOTOROUT1 17
#define MOTOROUT2 27
#define MOTORPWM 18
#define POW1 1024
#define POW2 768

#define JOY_DEV "/dev/input/js0"

using namespace std;

int main(int argc, char **argv) {
    ros::init(argc, argv, "ros_rightmotor");
    int old_vx = 0;

    // Initialize Motor
    if ( wiringPiSetupGpio() == -1) {
        printf("setup error");
        return -1;
    }

    pinMode(MOTOROUT1, OUTPUT);
    pinMode(MOTOROUT2, OUTPUT);
    pinMode(MOTORPWM, PWM_OUTPUT);

    digitalWrite(MOTOROUT1, 0);
    digitalWrite(MOTOROUT2, 0);
    digitalWrite(MOTORPWM, 0);

    
    // Initialize Joystick
    int joy_fd = -1;
    int num_of_axis = 0;
    int num_of_buttons = 0;

    char name_of_joystick[80];
    vector<char> joy_button;
    vector<int> joy_axis;
    
    if ((joy_fd = open(JOY_DEV, O_RDONLY)) < 0) {
        printf("Failed to open %s", JOY_DEV);
        return -1;
    }

    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

    joy_button.resize(num_of_buttons, 0);
    joy_axis.resize(num_of_axis, 0);
    
    printf("Joystick: %s axis: %d buttons: %d\n", name_of_joystick, num_of_axis, num_of_buttons);

    fcntl(joy_fd, F_SETFL, O_NONBLOCK); // using non-blocking mode

    while(true) {
        js_event js;

        read(joy_fd, &js, sizeof(js_event));
        
        switch(js.type & ~JS_EVENT_INIT) {
            case JS_EVENT_AXIS:
                joy_axis[(int)js.number] = js.value;
                break;
            case JS_EVENT_BUTTON:
                joy_button[(int)js.number] = js.value;
                break;
        }

        if ( num_of_axis > 0 ) {
            int now_vx = joy_axis[0] / 32;
            
            printf("vx %d old %d\n", now_vx, old_vx);
            
            if ( ((old_vx < 0) ^ (now_vx < 0)) != 0 ) {
                // ‰ñ“]•ûŒü‚ª•Ï‚í‚é‚Æ‚«‚Íˆêu’âŽ~‚µ‚È‚¢‚Æ‰ó‚ê‚é
                pwmWrite(MOTORPWM, 0);
                digitalWrite(MOTOROUT1, 0);
                digitalWrite(MOTOROUT2, 0);
                usleep(50);
            }

            if ( now_vx != 0 ) {
                bool bit = (now_vx > 0 );
                digitalWrite(MOTOROUT1, (int)bit);
                digitalWrite(MOTOROUT2, (int)(!bit));
                pwmWrite(MOTORPWM, now_vx);
            }   
            old_vx = now_vx;
        }

        usleep(100);
    }
    
    close(joy_fd);
    return 0;
}
