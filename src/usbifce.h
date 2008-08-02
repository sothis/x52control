#ifndef _X52PRO_H
#define _X52PRO_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct x52;

    enum x52_type {DEV_X52, DEV_X52PRO, DEV_YOKE};

#define X52PRO_LED_FIRE     1 /* RED/GREEN LED of Fire Button */
#define X52PRO_LED_ARED     2 /* RED LED of A Button */
#define X52PRO_LED_AGREEN   3 /* GREEN LED of A Button */
#define X52PRO_LED_BRED     4 /* RED LED of B Button */
#define X52PRO_LED_BGREEN   5 /* GREEN LED of B Button */
#define X52PRO_LED_DRED     6 /* RED LED of D Button */
#define X52PRO_LED_DGREEN   7 /* GREEN LED of D Button */
#define X52PRO_LED_ERED     8 /* RED LED of E Button */
#define X52PRO_LED_EGREEN   9 /* GREEN LED of E Button */
#define X52PRO_LED_T1RED   10 /* RED LED of T1 Switch */
#define X52PRO_LED_T1GREEN 11 /* GREEN LED of T1 Switch */
#define X52PRO_LED_T2RED   12 /* RED LED of T2 Switch */
#define X52PRO_LED_T2GREEN 13 /* GREEN LED of T2 Switch */
#define X52PRO_LED_T3RED   14 /* RED LED of T3 Switch */
#define X52PRO_LED_T3GREEN 15 /* GREEN LED of T3 Switch */
#define X52PRO_LED_CORED   16 /* RED LED of Coolie Hat */
#define X52PRO_LED_COGREEN 17 /* GREEN LED of Coolie Hat */
#define X52PRO_LED_IRED    18 /* RED LED of i Button */
#define X52PRO_LED_IGREEN  19 /* GREEN LED of i Button */
#define X52PRO_LED_THROTT  20 /* LED of Throttlescale  */

    struct x52* x52_init(void);
    void x52_debug(struct x52* hdl, int debug);
    enum x52_type x52_gettype(struct x52* hdl);
    void x52_close(struct x52* hdl);
    int x52_clearall(struct x52 *x52);
    int x52_cleartext(struct x52 *x52, int line);
    int x52_settext(struct x52 *hdl, int line, char *text, int length);
    int x52_setbri(struct x52 *hdl, int mfd, int brightness);
    int x52_setled(struct x52 *hdl, int led, int on);
    int x52_settime(struct x52 *hdl, int h24, int hour, int minute);
    int x52_setoffs(struct x52 *hdl, int idx, int h24, int inv, int offset);
    int x52_setsecond(struct x52 *x52, int second);
    int x52_setdate(struct x52 *hdl, int year, int month, int day);

#ifdef __cplusplus
}
#endif

#endif /* _X52PRO_H */
