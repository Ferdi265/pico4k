MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2048k
}

ENTRY(__flash_binary_entry)

SECTIONS
{
    /* Second stage bootloader is prepended to the image. It must be 256 bytes big
       and checksummed. It is usually built by the boot_stage2 target
       in the Raspberry Pi Pico SDK
    */

    .flash_begin : {
        __flash_binary_start = .;
        __flash_binary_entry = . + 1;
    } > FLASH

    .boot2 : {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ == 256,
        "ERROR: Pico second stage bootloader must be 256 bytes in size")

    .parts : {
        __parts_start__ = .;
        KEEP (*(.part*))
        __parts_end__ = .;
    } > FLASH

    .flash_end : {
        __flash_binary_end = .;
    } > FLASH
}
