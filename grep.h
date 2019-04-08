#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void commands(void);
void add(int i);
unsigned int *address(void);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
int backref(int i, char *lp);
void blkio(int b, char *buf, int (*iofcn)(int, char*, int));
void callunix(void);
int cclass(char *set, int c, int af);
void compile(int eof);
int compsub(void);
void dosub(void);  void error(char *s);
int execute(unsigned int *addr);
void exfile(void);
void filename(const char* c);
void gdelete(void);
char *getblock(unsigned int atl, int iof);
int getchr(void);
int getcopy(void);
int getfile(void);
char *mygetline(unsigned int tl);
int getnum(void);
int getsub(void);
int gettty(void);
int gety(void);
void global(int k);
void init(void);
void join(void);
void move_(int cflag);
void newline(void);
void nonzero(void);
void onhup(int n);
void onintr(int n);
char *place(char *sp, char *l1, char *l2);
void print(void);
void putchr_(int ac);
void putd(void);
void putfile(void);
int putline(void);
void puts_(char *sp);
void quit(int n);
void rdelete(unsigned int *ad1, unsigned int *ad2);
void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);
void setnoaddr(void);
void squeeze(int);
void substitute(int inglob);
void greperror(char);
void grepline(void);

void readfile(const char *c);
void ungetch_(int c);
void commands(void);
void printcommand(void);
void search(const char* c);
void process_dir(const char* dir, const char* searchfor, void(*fp)(const char*, const char*));
void search_file(const char* filename, const char* searhfor);

typedef void (*SIG_TYP)(int);
