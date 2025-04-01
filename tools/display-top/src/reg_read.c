// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <string.h>
// #include <fcntl.h>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <errno.h>
// #include <stdbool.h>

// #include "intel_reg_spec.h"



// void intel_mmio_use_dump_file(struct intel_mmio_data *mmio_data, char *file)
// {
// 	int fd;
// 	struct stat st;

// 	memset(mmio_data, 0, sizeof(struct intel_mmio_data));
// 	fd = open(file, O_RDWR);
// 	igt_fail_on_f(fd == -1,
// 		      "Couldn't open %s\n", file);

// 	fstat(fd, &st);
// 	mmio_data->igt_mmio = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
// 	igt_fail_on_f(mmio_data->igt_mmio == MAP_FAILED,
// 		      "Couldn't mmap %s\n", file);

// 	mmio_data->mmio_size = st.st_size;
// 	igt_global_mmio = mmio_data->igt_mmio;

// 	close(fd);
// }

// static void dump_register(struct config *config, struct reg *reg)
// {
// 	uint32_t val;

// 	if (read_register(config, reg, &val) == 0)
// 		dump_regval(config, reg, val);
// }


// static int read_register(struct config *config, struct reg *reg, uint32_t *valp)
// {
// 	uint32_t val = 0;

// 	switch (reg->port_desc.port) {
// 	case PORT_MCHBAR_32:
// 	case PORT_MMIO_32:
// 		if (reg->engine)
// 			val = register_srm(config, reg, NULL);
// 		else
// 			val = INREG(reg->mmio_offset + reg->addr);
// 		break;
// 	case PORT_MCHBAR_16:
// 	case PORT_MMIO_16:
// 		val = INREG16(reg->mmio_offset + reg->addr);
// 		break;
// 	case PORT_MCHBAR_8:
// 	case PORT_MMIO_8:
// 		val = INREG8(reg->mmio_offset + reg->addr);
// 		break;
// 	case PORT_MMIO_VGA_AR:
// 		val = vga_ar_read(reg->addr, true);
// 		break;
// 	case PORT_MMIO_VGA_SR:
// 		val = vga_sr_read(reg->addr, true);
// 		break;
// 	case PORT_MMIO_VGA_GR:
// 		val = vga_gr_read(reg->addr, true);
// 		break;
// 	case PORT_MMIO_VGA_CR:
// 		val = vga_cr_read(reg->addr, true);
// 		break;
// 	case PORT_PORTIO:
// 		iopl(3);
// 		val = inb(reg->addr);
// 		iopl(0);
// 		break;
// 	case PORT_PORTIO_VGA_AR:
// 		val = vga_ar_read(reg->addr, false);
// 		break;
// 	case PORT_PORTIO_VGA_SR:
// 		val = vga_sr_read(reg->addr, false);
// 		break;
// 	case PORT_PORTIO_VGA_GR:
// 		val = vga_gr_read(reg->addr, false);
// 		break;
// 	case PORT_PORTIO_VGA_CR:
// 		val = vga_cr_read(reg->addr, false);
// 		break;
// 	case PORT_BUNIT:
// 	case PORT_PUNIT:
// 	case PORT_NC:
// 	case PORT_DPIO:
// 	case PORT_GPIO_NC:
// 	case PORT_CCK:
// 	case PORT_CCU:
// 	case PORT_DPIO2:
// 	case PORT_FLISDSI:
// 		if (!IS_VALLEYVIEW(config->devid) &&
// 		    !IS_CHERRYVIEW(config->devid)) {
// 			fprintf(stderr, "port %s only supported on vlv/chv\n",
// 				reg->port_desc.name);
// 			return -1;
// 		}
// 		val = intel_iosf_sb_read(&config->mmio_data, reg->port_desc.port, reg->addr);
// 		break;
// 	default:
// 		fprintf(stderr, "port %d not supported\n", reg->port_desc.port);
// 		return -1;
// 	}

// 	if (valp)
// 		*valp = val;

// 	return 0;
// }


// static void dump_regval(struct config *config, struct reg *reg, uint32_t val)
// {
// 	char decode[1300];
// 	char tmp[1024];
// 	char bin[200];

// 	if (config->binary)
// 		to_binary(bin, sizeof(bin), val);
// 	else
// 		*bin = '\0';

// 	if (config->decode)
// 		intel_reg_spec_decode(tmp, sizeof(tmp), reg, val,
// 				      config->all_platforms ? 0 : config->devid);
// 	else
// 		*tmp = '\0';

// 	if (*tmp) {
// 		/* We have a decode result, and maybe binary decode. */
// 		if (config->all_platforms)
// 			snprintf(decode, sizeof(decode), "\n%s%s", tmp, bin);
// 		else
// 			snprintf(decode, sizeof(decode), " (%s)\n%s", tmp, bin);
// 	} else if (*bin) {
// 		/* No decode result, but binary decode. */
// 		snprintf(decode, sizeof(decode), "\n%s", bin);
// 	} else {
// 		/* No decode nor binary decode. */
// 		snprintf(decode, sizeof(decode), "\n");
// 	}

// 	if (port_is_mmio(reg->port_desc.port)) {
// 		/* Omit port name for MMIO, optionally include MMIO offset. */
// 		if (reg->mmio_offset)
// 			printf("%24s (0x%08x:0x%08x): 0x%08x%s",
// 			       reg->name ?: "",
// 			       reg->mmio_offset, reg->addr,
// 			       val, decode);
// 		else
// 			printf("%35s (0x%08x): 0x%08x%s",
// 			       reg->name ?: "",
// 			       reg->addr,
// 			       val, decode);
// 	} else {
// 		char name[100], addr[100];

// 		/* If no name, use addr as name for easier copy pasting. */
// 		if (reg->name)
// 			snprintf(name, sizeof(name), "%s:%s",
// 				 reg->port_desc.name, reg->name);
// 		else
// 			snprintf(name, sizeof(name), "%s:0x%08x",
// 				 reg->port_desc.name, reg->addr);

// 		/* Negative port numbers are not real sideband ports. */
// 		if (reg->port_desc.port > PORT_NONE)
// 			snprintf(addr, sizeof(addr), "0x%02x:0x%08x",
// 				 reg->port_desc.port, reg->addr);
// 		else
// 			snprintf(addr, sizeof(addr), "%s:0x%08x",
// 				 reg->port_desc.name, reg->addr);

// 		printf("%24s (%s): 0x%08x%s", name, addr, val, decode);
// 	}
// }


// static int intel_reg_read(struct config *config, int argc, char *argv[])
// {
// 	int i, j;

// 	if (argc == 1) {
// 		fprintf(stderr, "read: no registers specified\n");
// 		return EXIT_FAILURE;
// 	}

// 	if (config->mmiofile)
// 		intel_mmio_use_dump_file(&config->mmio_data, config->mmiofile);
// 	else
// 		intel_register_access_init(&config->mmio_data, config->pci_dev, 0, -1);

// 	for (i = 1; i < argc; i++) {
// 		struct reg reg;

// 		if (parse_reg(config, &reg, argv[i]))
// 			continue;

// 		for (j = 0; j < config->count; j++) {
// 			dump_register(config, &reg);
// 			/* Update addr and name. */
// 			set_reg_by_addr(config, &reg,
// 					reg.addr + reg.port_desc.stride);
// 		}
// 	}

// 	intel_register_access_fini(&config->mmio_data);

// 	return EXIT_SUCCESS;
// }
