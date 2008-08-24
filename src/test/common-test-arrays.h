
#define THOUSAND     1000
#define MILLION      1000000
#define BILLION      1000000000
#define TRILLION     1000000000000LL
#define QUADRILLION  1000000000000000LL
#define QUINTILLION  1000000000000000000LL

int64_t int64_roundnumbers[15] = { -QUINTILLION, -QUADRILLION, -TRILLION,
                                   -BILLION, -MILLION, -THOUSAND,
                                   1,
                                   THOUSAND, MILLION, BILLION,
                                   TRILLION, QUADRILLION, QUINTILLION };
int64_t int64_min_max[2] = { INT64_MIN, INT64_MAX };

uint64_t uint64_roundnumbers[9] = { 1,
                                   THOUSAND, MILLION, BILLION,
                                   TRILLION, QUADRILLION, QUINTILLION };
uint64_t uint64_0_1_max[3] = { 0, 1, UINT64_MAX };
uint64_t uint64_random[] = {0,
                          666,
                          4200000000ULL,
                          16ULL * (uint64_t) QUINTILLION + 33 };

#define FLOATING_POINT_RANDOM \
-1000.0, -100.0, -42.0, 0, 666, 131313
float float_random[] = { FLOATING_POINT_RANDOM };
double double_random[] = { FLOATING_POINT_RANDOM };

protobuf_c_boolean boolean_0[]  = {0 };
protobuf_c_boolean boolean_1[]  = {1 };
protobuf_c_boolean boolean_random[] = {0,1,1,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,1,1,0,1,1,0 };
