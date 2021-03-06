#include "port_io.h"

void render_1bit_image(unsigned char *screen, unsigned char *image, int width, int height, int x, int y, unsigned char color);

unsigned char font[] = {

  0, 1, 1, 0,
  1, 0, 0, 1,
  1, 0, 0, 1,
  1, 0, 0, 1,
  0, 1, 1, 0,


  0, 1, 1, 0,
  0, 0, 1, 0,
  0, 0, 1, 0,
  0, 0, 1, 0,
  0, 1, 1, 1,


  0, 1, 1, 0,
  0, 0, 1, 0,
  0, 1, 0, 0,
  0, 1, 0, 0,
  0, 1, 1, 0,


  0, 1, 1, 0,
  0, 0, 1, 0,
  0, 1, 1, 0,
  0, 0, 1, 0,
  0, 1, 1, 0,


  1, 0, 1, 0,
  1, 0, 1, 0,
  1, 1, 1, 1,
  0, 0, 1, 0,
  0, 0, 1, 0,


  0, 1, 1, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 1, 0,
  0, 1, 1, 0,


  0, 0, 1, 0,
  0, 1, 0, 0,
  0, 1, 1, 0,
  0, 1, 0, 1,
  0, 0, 1, 0,


  0, 1, 1, 1,
  0, 0, 0, 1,
  0, 0, 1, 0,
  0, 0, 1, 0,
  0, 1, 0, 0,


  0, 1, 1, 0,
  1, 0, 0, 1,
  0, 1, 1, 0,
  1, 0, 0, 1,
  0, 1, 1, 0,


  0, 1, 1, 1,
  0, 1, 0, 1,
  0, 1, 1, 1,
  0, 0, 0, 1,
  0, 1, 1, 0
};

char *a = "Hello world!";

int abs(int n) {
  return (n >= 0) ? n : -n;
}

// frequency is 1/frequency
int wave(int frequency, int amplitude, int t) {
  int loc = (t % frequency) - frequency/2;
  return abs(2*amplitude/frequency*loc);
}

/*int int_sqrt(int n) {

}

int sphere_sdf(int x, int y, int z, int radius) {
  return int_sqrt(x*x + y*y + z*z) - radius;
}*/

void init_fpu() {
  unsigned int cr4;

  // place CR4 into our variable
  __asm__ __volatile__("mov %%cr4, %0;" : "=r" (cr4));

  // set the OSFXSR bit
  cr4 |= 0x200;

  // reload CR4
  __asm__ __volatile__("mov %0, %%cr4;" : : "r"(cr4));

    // INIT the FPU (FINIT)
  __asm__ __volatile__("finit;");

  int cw = 0;

  // FLDCW = Load FPU Control Word
  asm volatile("fldcw %0;    "
               ::"m"(cw));     // sets the FPU control word to "cw"
}

float sqrt(float x) { // https://stackoverflow.com/questions/5000109/implement-double-sqrtdouble-x-in-c
  long i;
   float x2, y;
   const float threehalfs = 1.5F;

   x2 = x * 0.5F;
   y  = x;
   i  = * ( long * ) &y;                     // floating point bit level hacking [sic]
   i  = 0x5f3759df - ( i >> 1 );             // Newton's approximation
   y  = * ( float * ) &i;
   y  = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
   y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration
   y  = y * ( threehalfs - ( x2 * y * y ) ); // 3rd iteration

   return 1/y;
}

typedef struct {
  float x, y, z;
} Vec3;

typedef struct {
  int intersected;
  Vec3 p;
} Intersection;

Vec3 Vec3_new(float x, float y, float z) {
  Vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

Vec3 Vec3_add(Vec3 v1, Vec3 v2) {
  return Vec3_new(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
}

Vec3 Vec3_sub(Vec3 v1, Vec3 v2) {
  return Vec3_new(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
}

Vec3 Vec3_scale(Vec3 v, float s) {
  return Vec3_new(s*v.x, s*v.y, s*v.z);
}

float Vec3_magnitude(Vec3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vec3 Vec3_normalize(Vec3 v) {
  return Vec3_scale(v, 1.0/Vec3_magnitude(v));
}

Intersection Intersection_new(int intersected, Vec3 p) {
  Intersection in;
  in.intersected = intersected;
  in.p = p;
  return in;
}

float sphere_sdf(Vec3 p, int frame) {
  float radius = 10.5;
  return Vec3_magnitude(Vec3_sub(p, Vec3_new(0.0, -1.0+0.1*frame, 2.0))) - radius;
}

Intersection march(Vec3 p, Vec3 ray, int frame) {
  float t = 0.0;

  for (int i = 0; i < 16; i++) {
    float d = sphere_sdf(p, frame);

    if (d < 0.001 && d > -0.001) {
      return Intersection_new(i, p);
    }

    if (t > 10.0) { // Back clipping sphere
      break;
    }

    t += d;
    p = Vec3_add(p, Vec3_scale(ray, t));
  }

  return Intersection_new(-1, Vec3_new(0, 0, 0));
}

void get_vesa_mode_info() {
  __asm__ ("call 0x4000");
}

#define WIDTH 1024
#define HEIGHT 768

void kernel_main() {
  //get_vesa_mode_info();
  //unsigned int ebx;
  //__asm__ __volatile__("mov %%ebx, %0;" : "=r" (ebx));


  init_fpu();

  /*
  QEMU - 0xFD000000
  VirtualBox - 0xE000000
  Bochs - 0xE0000000
  */
  unsigned char *screen = (unsigned char *) 0xE0000000;

  Vec3 camera = Vec3_new(0.0, 0.0, -5.0);

  int frame = 0;
  while (1) {
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        screen[4*(y*WIDTH + x)] = 0xFF;
        //screen[3*(y*WIDTH + x)] = *((unsigned char *) 0x01000000);//(ebx>>16);
        /*Vec3 uv = Vec3_new(2.0*((float)x - WIDTH/2)/HEIGHT, 2.0*((float)y-HEIGHT/2)/HEIGHT, 0.0);
        Vec3 ray = Vec3_normalize(Vec3_sub(uv, camera));
        Intersection in = march(camera, ray, frame);
        int i = 3*(y*WIDTH + x);
        if (in.intersected != -1) {
          screen[i+2] = 0xFF;
          screen[i+1] = 0x00;
          screen[i+0] = 0x00;
        }
        else {
          screen[i+2] = 0x00;
          screen[i+1] = 0x00;
          screen[i+0] = 0x55;
        }*/
      }
    }
    frame++;
  }
}

void render_1bit_image(unsigned char *screen, unsigned char *image, int width, int height, int x, int y, unsigned char color) {
  for (int iy = 0; iy < height; iy++) { // iy = iterator y
    for (int ix = 0; ix < width; ix++) { // ix = iterator x
      int sx = x+ix; // sx = screen x
      int sy = y+iy; // sy = screen y
      if (0 <= sx && sx < WIDTH && 0 <= sy && sy < HEIGHT) {
        screen[sy*WIDTH + sx] = ((image[iy*width + ix] == 1) ? color : 0x00);
      }
    }
  }
}
