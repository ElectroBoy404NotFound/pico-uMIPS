add_executable(pico-uMIPS
    main.c
	
	psram/psram.c
	cache/cache.c

	console/console.c

	config/sd_hw_config.c

	emulator/cpu.c
	emulator/decBus.c
	emulator/ds1287.c
	emulator/dz11.c
	emulator/mem.c
	emulator/soc_rp2040.c

	sd/sd.c
)

# Add the standard library and FatFS/SPI to the build
target_link_libraries(pico-uMIPS
	pico_stdlib 
	FatFs_SPI 
	pico_time
	pico_multicore
	hardware_spi
	hardware_i2c
	hardware_vreg
	hardware_clocks
)

# pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
# target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)

# pico_set_boot_stage2(pico-uMIPS slower_boot2)

pico_set_binary_type(pico-uMIPS copy_to_ram)
pico_add_extra_outputs(pico-uMIPS)

