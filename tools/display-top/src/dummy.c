#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pciaccess.h>
#include <unistd.h>

// MMIO space pointer
volatile uint8_t *mmio;

// Function to read a 32-bit register
uint32_t mmio_readl(uint32_t offset) {
    return *(volatile uint32_t *)(mmio + offset);
}

// Function to find the Intel GPU
struct pci_device *find_intel_gpu() {
    struct pci_device_iterator *iter;
    struct pci_device *dev;

    pci_system_init(); // Initialize PCI access
    iter = pci_slot_match_iterator_create(NULL);

    while ((dev = pci_device_next(iter)) != NULL) {
        if (dev->vendor_id == 0x8086) { // Intel vendor ID
            pci_iterator_destroy(iter);
            return dev;
        }
    }

    pci_iterator_destroy(iter);
    return NULL;
}

int dummy(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <register_offset>\n", argv[0]);
        return EXIT_FAILURE;
    }

    uint32_t reg_offset = strtoul(argv[1], NULL, 0);
    struct pci_device *dev;
    uint64_t mmio_base;
    uint32_t mmio_size;

    // Find Intel GPU
    dev = find_intel_gpu();
    if (!dev) {
        fprintf(stderr, "Intel GPU not found!\n");
        return EXIT_FAILURE;
    }

    // Enable PCI device
    pci_device_probe(dev);

    // Get MMIO base address (BAR 0)
    mmio_base = dev->regions[0].base_addr;
    mmio_size = dev->regions[0].size;

    // Map MMIO space
    if (pci_device_map_range(dev, mmio_base, mmio_size, PCI_DEV_MAP_FLAG_WRITABLE, (void **)&mmio) != 0) {
        fprintf(stderr, "Failed to map MMIO space.\n");
        return EXIT_FAILURE;
    }

    // Read and print register value
    uint32_t value = mmio_readl(reg_offset);
    printf("Register 0x%08x: 0x%08x\n", reg_offset, value);

    // Unmap and cleanup
    pci_device_unmap_range(dev, (void *)mmio, mmio_size);
    pci_system_cleanup();

    return EXIT_SUCCESS;
}
