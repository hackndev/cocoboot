#define FLD_MASK 3
#define FLD_INVALID 0
#define FLD_COARSE 1
#define FLD_SECTION 2
#define FLD_FINE 3

UInt32 get_ttb();
UInt32 get_virt_ttb();
UInt32 get_ram_size();
UInt32 get_ram_base();
UInt32 get_reported_ram_size();
UInt32 virt_to_phys(UInt32 virt);
UInt32 phys_to_virt(UInt32 virt);
UInt32 map_mem(UInt32 pa);
