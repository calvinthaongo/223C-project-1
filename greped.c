#include "grep.h"

const int BLKSIZE = 40960;
const int NBLK = 2047;
const int FNSIZE = 128;
const int LBSIZE = 40960;
const int ESIZE = 256;
const int GBSIZE = 256;
const int NBRA = 5;
const int KSIZE = 9;
const int CBRA = 1;
const int CCHR = 2;
const int CDOT = 4;
const int CCL = 6;
const int NCCL = 8;
const int CDOL = 10;
const int CEOF = 11;
const int CKET = 12;
const int CBACK = 14;
const int CCIRC = 15;
const int STAR = 01;
const int READ = 0;
const int WRITE = 1;
const int BUFSIZE = 100;

int  peekc;
int  lastc;
int  given;
int  ninbuf;
int  io;
int  pflag;
int  vflag  = 1;
int  oflag;
int  listf;
int  listn;
int  col;
int  tfile  = -1;
int  tline;
int  iblock  = -1;
int  oblock  = -1;
int  ichanged;
int  nleft;
int  names[26];
int  anymarks;
int  nbra;
int  subnewa;
int  subolda;
int  fchange;
int  wrapp;
unsigned nlall = 128;
unsigned int *addr1;
unsigned int *addr2;
unsigned int *dot;
unsigned int *dol;
unsigned int *zero;

char inputbuf[GBSIZE];
long  count;
jmp_buf  savej;

char Q[] = "";
char T[] = "TMP";
char savedfile[FNSIZE];
char file[FNSIZE];
char linebuf[LBSIZE];
char rhsbuf[LBSIZE/2];
char expbuf[ESIZE+4];
char  genbuf[LBSIZE];
char *nextip;
char *linebp;
char *globp;
char *mktemp(char *);
char tmpXXXXX[50] = "/tmp/eXXXXX";
char  *tfname;
char *loc1;
char *loc2;
char ibuff[BLKSIZE];
char obuff[BLKSIZE];
char WRERR[]  = "WRITE ERROR";
char *braslist[NBRA];
char *braelist[NBRA];
char  line[70];
char  *linp  = line;
char grepbuf[GBSIZE];
char buf[BUFSIZE];
int bufp = 0;


int main(int argc, const char *argv[]) {
  zero = (unsigned *)malloc(nlall * sizeof(unsigned));
  tfname = mkdtemp(tmpXXXXX);
  if (argc != 3) {
    printf(". . . Syntax Error . . .");
    exit(1);
  }
  search_file(argv[2], argv[1]);
  return 0;
}
void filename(const char* c) {
  strcpy(file, c);
  strcpy(savedfile, c);
}
void myprint(void) {
  int c;
  char lastsep;
 unsigned int* a1;
  c = '\n';
    for (addr1 = 0;;) {
      lastsep = c;
      a1 = address();
      c = getchr();
      if (c != ',' && c != ';') {
         break;
       }
      addr1 = a1;
      if (c == ';') {
        dot = a1;
      }
    }
    if (lastsep != '\n' && a1 == 0) {
      a1 = dol;
    }
    if ((addr2 = a1)==0) {
      given = 0;
      addr2 = dot;
    }
    else {
      given = 1;
    }
    if (addr1==0) {
       addr1 = addr2;

    switch(c) {
        case 'p':
        case 'P':
          print();
          break;

        case EOF:
         default:
         return;
    }
  }
}
void search_file(const char* filename_, const char* searchfor) {
  setnoaddr();
  filename(filename_);
  init();
  if ((io = open((const char*)file, 0)) < 0) {
    lastc = '\n';
    error(file);
  }
  setwide();
  squeeze(0);
  append(getfile, addr2);
  exfile();
  fchange = *filename_;
  char buf[GBSIZE];
  snprintf(buf, sizeof(buf), "/%s\n", searchfor);
  const char* p = buf + strlen(buf) - 1;
  while (p >= buf) {
    ungetch_(*p--);
  }
  global(1);
}
int  getch_(void) {
  return (bufp > 0) ? buf[--bufp] : getchar();
}
void ungetch_(int c) {
  if (bufp >= BUFSIZE)
    printf("ungetch: too many chars\n");
  else
    buf[bufp++] = c;
}
unsigned int* address(void) {
  int sign;
  	unsigned int *a, *b;
  	int opcnt, nextopand;
  	int c;

  	nextopand = -1;
  	sign = 1;
  	opcnt = 0;
  	a = dot;
  	do {
  		do c = getchr(); while (c==' ' || c=='\t');
  		if ('0'<=c && c<='9') {
  			peekc = c;
  			if (!opcnt)
  				a = zero;
  		} else switch (c) {
  		case '$':
  			a = dol;
  			/* fall through */
  		case '.':
  			if (opcnt)
  				error(Q);
  			break;
  		case '\'':
  			c = getchr();
  			if (opcnt || c<'a' || 'z'<c)
  				error(Q);
  			a = zero;
  			do a++; while (a<=dol && names[c-'a']!=(*a&~01));
  			break;
  		case '?':
  			sign = -sign;
  			/* fall through */
  		case '/':
  			compile(c);
  			b = a;
  			for (;;) {
  				a += sign;
  				if (a<=zero)
  					a = dol;
  				if (a>dol)
  					a = zero;
  				if (execute(a))
  					break;
  				if (a==b)
  					error(Q);
  			}
  			break;
  		default:
  			if (nextopand == opcnt) {
  				a += sign;
  				if (a<zero || dol<a)
  					continue;       /* error(Q); */
  			}
  			if (c!='+' && c!='-' && c!='^') {
  				peekc = c;
  				if (opcnt==0)
  					a = 0;
  				return (a);
  			}
  			sign = 1;
  			if (c!='+')
  				sign = -sign;
  			nextopand = ++opcnt;
  			continue;
  		}
  		sign = 1;
  		opcnt++;
  	} while (zero<=a && a<=dol);
  	error(Q);
  	/*NOTREACHED*/
  	return 0;
}
int advance(char *lp, char *ep) {
	char *curlp;
	int i;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		loc2 = lp;
		return(1);

	case CCL:
		if (cclass(ep, *lp++, 1)) {
			ep += *ep;
			continue;
		}
		return(0);

	case NCCL:
		if (cclass(ep, *lp++, 0)) {
			ep += *ep;
			continue;
		}
		return(0);

	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		if (braelist[i = *ep++]==0)
			error(Q);
		return(0);

	case CBACK|STAR:
		if (braelist[i = *ep++] == 0)
			error(Q);
		curlp = lp;
		while (lp >= curlp) {
			if (advance(lp, ep))
				return(1);
			lp -= braelist[i] - braslist[i];
		}
		continue;

	case CDOT|STAR:
		curlp = lp;
		while (*lp++)
			;

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep)
			;
		ep++;

	case CCL|STAR:
	case NCCL|STAR:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)))
			;
		ep += *ep;

	default:
		error(Q);
	}
}
int append(int (*f)(void), unsigned int *a) {
	unsigned int *a1, *a2, *rdot;
	int nline, tl;

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if ((dol-zero)+1 >= nlall) {
			unsigned *ozero = zero;

			nlall += 1024;
			if ((zero = (unsigned *)realloc((char *)zero, nlall*sizeof(unsigned)))==NULL) {
				error("MEM?");
			}
			dot += zero - ozero;
			dol += zero - ozero;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}
void blkio(int b, char *buf, int (*iofcn)(int, char*, int)) {
	lseek(tfile, (long)b*BLKSIZE, 0);
	if ((*iofcn)(tfile, buf, BLKSIZE) != BLKSIZE) {
		error(T);
	}
}
int cclass(char *set, int c, int af) {
	int n;

	if (c==0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}
void compile(int eof) {
	int c;
	char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int cclcnt;

	ep = expbuf;
	bracketp = bracket;
	if ((c = getchr()) == '\n') {
		peekc = c;
		c = eof;
	}
	if (c == eof) {
		if (*ep==0)
			error(Q);
		return;
	}
	nbra = 0;
	if (c=='^') {
		c = getchr();
		*ep++ = CCIRC;
	}
	peekc = c;
	lastep = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE]){}
		c = getchr();
		if (c == '\n') {
			peekc = c;
			c = eof;
		}
		if (c==eof) {
			if (bracketp != bracket){}
			*ep++ = CEOF;
			return;
		}
		if (c!='*')
			lastep = ep;
		switch (c) {

		case '\\':
			if ((c = getchr())=='(') {
				if (nbra >= NBRA){}
				*bracketp++ = nbra;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket){}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c>='1' && c<'1'+NBRA) {
				*ep++ = CBACK;
				*ep++ = c-'1';
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n'){}
			*ep++ = c;
			continue;

		case '.':
			*ep++ = CDOT;
			continue;

		case '\n':{}

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET){}
			*lastep |= STAR;
			continue;

		case '$':
			if ((peekc=getchr()) != eof && peekc!='\n'){}
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchr()) == '^') {
				c = getchr();
				ep[-2] = NCCL;
			}
			do {
				if (c=='\n'){}
				if (c=='-' && ep[-1]!=0) {
					if ((c=getchr())==']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
						if (ep>=&expbuf[ESIZE])
            {}
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE]){}
			} while ((c = getchr()) != ']');
			lastep[1] = cclcnt;
			continue;

		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}
void error(char *s) {
	int c;

	wrapp = 0;
	listf = 0;
	listn = 0;
	puts(s);
	count = 0;
	lseek(0, (long)0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getchr()) != '\n' && c != EOF)
			;
	if (io > 0) {
		close(io);
		io = -1;
	}
	longjmp(savej, 1);
}
int execute(unsigned int *addr) {
	char *p1, *p2;
	int c;

	for (c=0; c<NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	p2 = expbuf;
	if (addr == (unsigned *)0) {
		if (*p2==CCIRC)
			return(0);
		p1 = loc2;
	} else if (addr==zero)
		return(0);
	else
		p1 = mygetline(*addr);
	if (*p2==CCIRC) {
		loc1 = p1;
		return(advance(p1, p2+1));
	}
	/* fast check for first character */
	if (*p2==CCHR) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}
	/* regular algorithm */
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}
void exfile(void) {
	close(io);
	io = -1;
}
char *getblock(unsigned int atl, int iof) {
	int bno, off;

	bno = (atl/(BLKSIZE/2));
	off = (atl<<1) & (BLKSIZE-1) & ~03;
	if (bno >= NBLK) {
		lastc = '\n';
		error(T);
	}
	nleft = BLKSIZE - off;
	if (bno==iblock) {
		ichanged |= iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==READ) {
		if (ichanged)
			blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}
int getchr(void) {
  char c;
  if ((lastc=peekc)) {
    peekc = 0;
    return(lastc);
  }
  if (globp) {
    if ((lastc = *globp++) != 0) {
      return(lastc);
    }
    globp = 0;
    return(EOF);
  }
  if ((c = getch_()) <= 0) {
    return(lastc = EOF);
  }
  lastc = c&0177;
  return(lastc);
}
int getfile(void) {
	int c;
	char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
				if (lp>linebuf) {
					*genbuf = '\n';
				}
				else return(EOF);
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) {
				if (*fp++ & 0200)
					break;
			}
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0')
			continue;
		if (c&0200 || lp >= &linebuf[LBSIZE]) {
			lastc = '\n';
			error(Q);
		}
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}
char* mygetline(unsigned int tl) {
  char *bp = getblock(tl, READ);
  char *lp = linebuf;
  int nl = nleft;
  tl &= ~((BLKSIZE/2)-1);
  while ((*lp++ = *bp++)) { if (--nl == 0) {  bp = getblock(tl+=(BLKSIZE/2), READ);  nl = nleft;  } }
  return(linebuf);
}
void global(int k) {
  char *gp;
  int c;
  unsigned int *a1;
  char globuf[GBSIZE];
  if (globp) {
    error(Q);
  }
  setwide();
  squeeze(dol > zero);
  if ((c = getchr()) == '\n') {
    error(Q);
  }
  compile(c);
  gp = globuf;
  while ((c = getchr()) != '\n') {
    if (c == EOF) {
       error(Q);
     }
    if (c == '\\') {
      c = getchr();
      if (c != '\n') {
        *gp++ = '\\';
      }
    }
    *gp++ = c;
    if (gp >= &globuf[GBSIZE-2]) {
      error(Q);
    }
  }
  if (gp == globuf) {
    *gp++ = 'p';
  }
  *gp++ = '\n';
  *gp++ = 0;
  for (a1 = zero; a1 <= dol; a1++) {
    *a1 &= ~01;
    if (a1>=addr1 && a1<=addr2 && execute(a1)==k) {
      *a1 |= 01;
    }
  }
  for (a1 = zero; a1 <= dol; a1++) {
    if (*a1 & 01) {
      *a1 &= ~01;
      dot = a1;
      globp = globuf;
      myprint();
      a1 = zero; }
  }
}
void init(void) {
	int *markp;

	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; )
		*markp++ = 0;
	subnewa = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	dot = dol = zero;
}
void nonzero(void) { squeeze(1); }
void print(void) {
	unsigned int *a1;

	nonzero();
	a1 = addr1;
	do {
		if (listn) {
			count = a1-zero;
		}
		puts(mygetline(*a1++));
	} while (a1 <= addr2);
	dot = addr2;
	listf = 0;
	listn = 0;
	pflag = 0;
}
void putchr_(int ac) {
	char *lp;
	int c;

	lp = linp;
	c = ac;
	if (listf) {
		if (c=='\n') {
			if (linp!=line && linp[-1]==' ') {
				*lp++ = '\\';
				*lp++ = 'n';
			}
		} else {
			if (col > (72-4-2)) {
				col = 8;
				*lp++ = '\\';
				*lp++ = '\n';
				*lp++ = '\t';
			}
			col++;
			if (c=='\b' || c=='\t' || c=='\\') {
				*lp++ = '\\';
				if (c=='\b')
					c = 'b';
				else if (c=='\t')
					c = 't';
				col++;
			} else if (c<' ' || c=='\177') {
				*lp++ = '\\';
				*lp++ =  (c>>6)    +'0';
				*lp++ = ((c>>3)&07)+'0';
				c     = ( c    &07)+'0';
				col += 3;
			}
		}
	}
	*lp++ = c;
	if(c == '\n' || lp >= &line[64]) {
		linp = line;
		write(oflag?2:1, line, lp-line);
		return;
	}
	linp = lp;
}
void putfile(void) {
	unsigned int *a1;
	int n;
	char *fp, *lp;
	int nib;

	nib = BLKSIZE;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = mygetline(*a1++);
		for (;;) {
			if (--nib < 0) {
				n = fp-genbuf;
				if(write(io, genbuf, n) != n) {
					puts(WRERR);
					error(Q);
				}
				nib = BLKSIZE-1;
				fp = genbuf;
			}
			count++;
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	n = fp-genbuf;
	if(write(io, genbuf, n) != n) {
		puts(WRERR);
		error(Q);
	}
}
int putline(void) {
	char *bp, *lp;
	int nl;
	unsigned int tl;

	fchange = 1;
	lp = linebuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl &= ~((BLKSIZE/2)-1);
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl+=(BLKSIZE/2), WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}
void setnoaddr(void) { if (given) { error(Q); } }
void setwide(void) { if (!given) { addr1 = zero + (dol>zero);  addr2 = dol; } }
void squeeze(int i) { if (addr1 < zero+i || addr2 > dol || addr1 > addr2) { error(Q); } }
