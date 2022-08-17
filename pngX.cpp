#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>
namespace std {}  
using namespace std;

typedef unsigned char U8;
typedef unsigned long U32;  

#define Top_value U32(0XFFFFFFFF)    
#define First_qtr ((Top_value>>2)+1) 
#define Half      U32(2*First_qtr)    
#define Third_qtr U32(3*First_qtr)    

int EOS = 0; 
U32 rc0,rc,rc1,rc2,rc3,r1,r2,r3,r4,ubi,xmid,cobit;
unsigned short buf[16777216];

  unsigned char cbuf[65793];
  int cxt,bcxt;  
  unsigned short ct[65793][256][2];  
  unsigned short bct[65536][256][2];
 
  unsigned long filesize,pos,control,i,lbit,posix,limbuf;
  int p() {
    return 4096*(ct[rc][cxt][1]+1) / (ct[rc][cxt][0]+ct[rc][cxt][1]+2);
  }

  void update(int y) {
    if (++ct[rc][cxt][y] > 248)
    {
      ct[rc][cxt][0] >>= 1;       ct[rc][cxt][0] +=15;
      ct[rc][cxt][1] >>= 1;       ct[rc][cxt][1] +=15;
    }

    if (ct[rc0][cxt][0]==0 && ct[rc0][cxt][1]==0) 
    {
	   if (ct[rc1][cxt][0]>0 || ct[rc1][cxt][1]>0 )
       {
		ct[rc0][cxt][1]<<=1;
       }
    }
  }



typedef enum {COMPRESS, DECOMPRESS} Mode;
class Encoder {
private:

  const Mode mode;       
  FILE* archive;         
  U32 x1, x2;            
  U32 x;                 
  U32 bits_to_follow;
  U8 bptr,bout,bptrin;
  int bin;
public:
  Encoder(Mode m, FILE* f);
  void encode(int y);    
  int decode();          
  void flush();          
  void bit_plus_follow(int bit);
  int input_bit(void);
};


inline void Encoder::bit_plus_follow(int bit)
{
    bits_to_follow++;
    for (int notb=bit^1; bits_to_follow > 0; bits_to_follow--, bit=notb) {
        if (bit) bout|=bptr;
        if (!(bptr>>=1)) {
            putc(bout,archive);
            bptr=128;
            bout=0;
        }
    }
}
inline int Encoder::input_bit(void)
{
    if (!(bptrin>>=1)) {
        bin=getc(archive);
        if (bin==EOF) {
                   bin=0;
                   EOS++;
                }
        bptrin=128;
    }
    return ((bin&bptrin)!=0);
}

Encoder::Encoder(Mode m, FILE* f): mode(m), archive(f), x1(0),
                                   x2(0xffffffff), x(0), bits_to_follow(0),
bptr(128),
bout(0), bptrin(1) {
  if (mode==DECOMPRESS) {
    x = 1;
    for (;x < Half;) x += x + input_bit();
    x += x + input_bit();
  }
}


inline void Encoder::encode(int y) {

  if (ubi==0)
  xmid = x1 + ((x2-x1) >> 12) * p();
  assert(xmid >= x1 && xmid < x2);
  if (y)
    x2=xmid;
  else
    x1=xmid+1;
  if (ubi==0)
  update(y);

  cobit--;
  if (cobit==0) cobit=8;

  for (;;) {
    if ( x2 < Half ) {
      bit_plus_follow(0);
    } else if (x1 >= Half) {
      bit_plus_follow(1);
    } else if (x1 >= First_qtr && x2 < Third_qtr) {
      bits_to_follow++;
      x1 ^= First_qtr;
      x2 ^= First_qtr;
    } else {
      break;
    }
    x1 += x1;
    x2 += x2 + 1;
  }
}

inline int Encoder::decode() {

  if (ubi==0)
  xmid = x1 + ((x2-x1) >> 12) * p();
  assert(xmid >= x1 && xmid < x2);
  int y=0;
  if (x<=xmid) {
    y=1;
    x2=xmid;
  }
  else
    x1=xmid+1;
  if (ubi==0)
  update(y);

  for (;;) {
      if ( x2 < Half ) {
      } else if (x1 >= Half) {    
     x1 -= Half;
     x -= Half;
     x2 -= Half;        
      } else if (x1 >= First_qtr    
         && x2 < Third_qtr) {    
     x1 -= First_qtr;    
     x -=  First_qtr;
     x2 -= First_qtr;
      } else {
     break;            
      }
      x1 += x1;
      x += x + input_bit();
      x2 += x2 + 1;    
      if ( EOS > 6 || ( EOS > 0 && ( x << 2) == 0 )) {
            EOS = 100;
            if ( x == Half || x == 0 ) EOS = 10;
      }
   }
  return y;
}

void Encoder::flush() {
    if (ubi==0)
  xmid = x1 + ((x2-x1) >> 12) * p();
  assert(xmid >= x1 && xmid < x2);
  if ( xmid < Half )
    x2=xmid;
  else
    x1=xmid+1;
  for (;;) {
    if ( x2 < Half ) {
      bit_plus_follow(0);
    } else if (x1 >= Half) {
      bit_plus_follow(1);
    } else if (x1 >= First_qtr && x2 < Third_qtr) {
      bits_to_follow++;
      x1 ^= First_qtr;
      x2 ^= First_qtr;
    } else {
      break;
    }
    x1 += x1;
    x2 += x2 + 1;
  }
   if ( x1 <=  First_qtr ) {
     bit_plus_follow(0);
     bit_plus_follow(1);
   } else {
     bit_plus_follow(1);
     bit_plus_follow(1);
   }
   if (bout) putc(bout,archive);
}

int main(int argc, char** argv) {

   if (argc!=4 || (argv[1][0]!='c' && argv[1][0]!='d')) {
    printf("To compress:   pngX c input output\n"
           "To decompress: pngX d input output\n");
    exit(1);
  }

  // Start timer
  clock_t start = clock();

  // Open files
  FILE *in=fopen(argv[2], "rb");
  if (!in) perror(argv[2]), exit(1);
  FILE *out=fopen(argv[3], "wb");
  if (!out) perror(argv[3]), exit(1);
  int c;
  int ok;
  cobit=8;
    cxt=1;
  memset(ct, 0, sizeof(ct));
      bcxt=1;
  memset(bct, 0, sizeof(bct));



  // Compress
  if (argv[1][0]=='c') {
    fseek (in , 0 , SEEK_END);
    filesize = ftell (in);
    rewind (in);
    fseek(out,0,SEEK_SET);
    fwrite (&filesize , 1 , 4 , out);
    Encoder e(COMPRESS, out);
    while ((c=getc(in))!=EOF) {
      ok=0;
      if (buf[r3+(r2<<8)+(r1<<16)]>0 && (buf[r3+(r2<<8)+(r1<<16)]-1)==c) ok=1; 
      ubi=1;
      if (buf[r3+(r2<<8)+(r1<<16)]>0) e.encode(ok); 
      if (ok==0) 
      {
      ubi=0;
      for (int i=7; i>=0; --i)
      e.encode((c>>i)&1); 
      }
        r1=c;
        rc1=1+r1;
        if (cbuf[rc1]>0) 
        rc=rc1;
        else
        cbuf[rc1]=1;
        
		   
    }
    e.flush();
  }

  else {
    fseek(in,0,SEEK_SET);
    fread (&filesize , 1 , 4 , in);
    Encoder e(DECOMPRESS, in);
    int c=1;
    while (filesize>0)
    {
      ubi=1;
      ok=0;
      if (buf[r3+(r2<<8)+(r1<<16)]>0) ok=e.decode();
      if (ok==0)
      {
      ubi=0;
      c = 1;
      while (c<256)
      {
        c+=c+e.decode();
        if ( EOS > 5 ) break;
      }
      if ( EOS > 5 ) break;
      putc(c-256, out);
      }
      else
      {
      putc(buf[r3+(r2<<8)+(r1<<16)]-1, out);
      c=(buf[r3+(r2<<8)+(r1<<16)]-1)+256;
      
	  }
        r1=c-256;
        rc1=1+r1;
        if (cbuf[rc1]>0)
        rc=rc1;
        else
        cbuf[rc1]=1;

        filesize--;
    }
  }

  // Print results
  printf("%s (%ld bytes) -> %s (%ld bytes) in %1.2f s.\n",
    argv[2], ftell(in), argv[3], ftell(out),
    ((double)clock()-start)/CLOCKS_PER_SEC);
  return 0;
}
