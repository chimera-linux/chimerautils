#ifndef GETCAP_H
#define GETCAP_H

char *cgetcap(char *, const char *, int);
int cgetent(char **, char **, const char *);
int cgetmatch(const char *, const char *);
int cgetnum(char *, const char *, long *);
int cgetset(const char *);
int cgetstr(char *, const char *, char **);

#endif
