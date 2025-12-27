#ifndef VANITYGAPS_H
#define VANITYGAPS_H

/* Vanity gaps functionality */
void setgaps(int oh, int ov, int ih, int iv);
void togglegaps(const Arg *arg);
void defaultgaps(const Arg *arg);
void incrgaps(const Arg *arg);
void incrigaps(const Arg *arg);
void incrogaps(const Arg *arg);
void incrohgaps(const Arg *arg);
void incrovgaps(const Arg *arg);
void incrihgaps(const Arg *arg);
void incrivgaps(const Arg *arg);

/* Layout functions with gaps */
void tile(Monitor *m);
void spiral(Monitor *m);
void dwindle(Monitor *m);

/* Internal functions */
void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc);
void getfacts(Monitor *m, int msize, int ssize, float *mf, float *sf, int *mr, int *sr);

#endif /* VANITYGAPS_H */
