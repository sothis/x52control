#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL_sysjoystick.h"

int SDL_PrivateJoystickAxis(SDL_Joystick *joystick, Uint8 axis, Sint16 value)
{
    joystick->axes[axis] = value;
    return 1;
}

int SDL_PrivateJoystickBall(SDL_Joystick *joystick, Uint8 ball, Sint16 xrel, Sint16 yrel)
{
    joystick->balls[ball].dx += xrel;
	joystick->balls[ball].dy += yrel;
    return 1;
}

int SDL_PrivateJoystickHat(SDL_Joystick *joystick, Uint8 hat, Uint8 value)
{
    joystick->hats[hat] = value;
    return 1;
}

int SDL_PrivateJoystickButton(SDL_Joystick *joystick, Uint8 button, Uint8 state)
{
    joystick->buttons[button] = state;
    printf("button event: but %d state %d\n", button, state);
    return 1;
}

int main(int argc, char* argv[])
{
    SDL_Joystick js;
    js.index = 0;
    int num_js = SDL_SYS_JoystickInit();
    
    printf("found : %d\n", num_js);
    if (!num_js)
        return 1;
    printf("name  : %s\n\n", SDL_SYS_JoystickName(0));
    
    SDL_SYS_JoystickOpen(&js);
    
    js.axes = malloc(js.naxes*sizeof(Sint16));
    memset(js.axes, 0, sizeof(Sint16));
    js.hats = malloc(js.nhats*sizeof(Uint8));
    memset(js.hats, 0, sizeof(Uint8));
    js.balls = malloc(js.nballs*sizeof(*js.balls));
    memset(js.balls, 0, sizeof(*js.balls));
    js.buttons = malloc(js.nbuttons*sizeof(Uint8));
    memset(js.buttons, 0, sizeof(Uint8));
    
    printf("axes    : %d\n", js.naxes);
    printf("hats    : %d\n", js.nhats);
    printf("balls   : %d\n", js.nballs);
    printf("buttons : %d\n", js.nbuttons);
    
    while(1)
    {
        SDL_SYS_JoystickUpdate(&js);
        usleep(500000);
    }
    
    SDL_SYS_JoystickClose(&js);
    SDL_SYS_JoystickQuit();
    return 0;
}