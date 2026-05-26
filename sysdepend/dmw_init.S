/* Direct-map windows (Linux head.S style) for MMIO/RAM on LA64 */
#define CSR_DMWIN0  0x180
#define CSR_DMWIN1  0x181
#define CSR_DMW0_INIT  0x8000000000000001
#define CSR_DMW1_INIT  0x9000000000000011

    .text
    .globl la64_dmw_init
la64_dmw_init:
    li.d    $t0, CSR_DMW0_INIT
    csrwr   $t0, CSR_DMWIN0
    li.d    $t0, CSR_DMW1_INIT
    csrwr   $t0, CSR_DMWIN1
    jirl    $zero, $ra, 0
