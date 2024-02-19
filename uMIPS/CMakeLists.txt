add_executable(pico-uMIPS
    main.c
	
	psram/psram.c
	psram/cache.c
	psram/cache_l2.c
	psram/icache.c
	
	console/console.c

	config/sd_hw_config.c

	# Emulator
	emulator/cpu/cpu.c
	emulator/bus/decBus.c
	emulator/rtc/ds1287.c
	emulator/dz11/dz11.c
	emulator/memory/mem.c
	emulator/soc/soc_rp2040.c
	emulator/fpu/fpu.c

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

