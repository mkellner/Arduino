// (c) 2014 mkellner@robotranch.org

#ifndef __DIRSWITCH_H__
#define __DIRSWITCH_H__ 1

#define SW_UP    8    // up selector switch
#define SW_DOWN  4    // down selector switch

extern char switchPos;
extern char dir;

char checkSwitch();  // returns 1 if the position has changed


#endif // __DIRSWITCH_H__
