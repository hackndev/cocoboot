#define CPU_VENDOR_MASK 0xff000000
#define CPU_MODEL_MASK 0xff000fff

/* Note: These CPU IDs are from TCPMP. Most have been confirmed by people testing 
 *       in the Hack&Dev forum.
 */

/* CPU Vendors */
#define CPUV_INTEL 0x69000000
#define CPUV_TI 0x54000000
#define CPUV_ARM 0x41000000

/* Intel CPUs */
#define CPU_SA1100 0x69000B11 /* StrongArm */
#define CPU_PXA25X 0x69000010 /* XScale */
#define CPU_PXA27X 0x69000011
#define CPU_PXA210 0x69000012

/* TI CPUs */
#define CPU_915T 0x54000915
#define CPU_925T 0x54000925
#define CPU_926T 0x54000926

/* ARM CPUs */
#define CPU_920T 0x41000920
#define CPU_922T 0x41000922
#define CPU_926E 0x41000926
#define CPU_940T 0x41000940
#define CPU_946E 0x41000946
#define CPU_1020E 0x41000A22

const char *get_cpu_vendor(UInt32 cpu);
const char *get_cpu_name(UInt32 cpu);
UInt32 get_cpu();
UInt32 get_cpu_id();
UInt32 get_dev_id();
