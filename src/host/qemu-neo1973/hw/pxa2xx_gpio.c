/*
 * Intel XScale PXA255/270 GPIO controller emulation.
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Written by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This code is licensed under the GPL.
 */

#include "vl.h"

#define PXA2XX_GPIO_BANKS	4

struct pxa2xx_gpio_info_s {
    target_phys_addr_t base;
    void *pic;
    int lines;
    CPUState *cpu_env;

    /* XXX: GNU C vectors are more suitable */
    uint32_t ilevel[PXA2XX_GPIO_BANKS];
    uint32_t olevel[PXA2XX_GPIO_BANKS];
    uint32_t dir[PXA2XX_GPIO_BANKS];
    uint32_t rising[PXA2XX_GPIO_BANKS];
    uint32_t falling[PXA2XX_GPIO_BANKS];
    uint32_t status[PXA2XX_GPIO_BANKS];
    uint32_t gafr[PXA2XX_GPIO_BANKS * 2];

    uint32_t prev_level[PXA2XX_GPIO_BANKS];
    struct {
        gpio_handler_t fn;
        void *opaque;
    } handler[PXA2XX_GPIO_BANKS * 32];

    void (*read_notify)(void *opaque);
    void *opaque;
};

static struct {
    enum {
        GPIO_NONE,
        GPLR,
        GPSR,
        GPCR,
        GPDR,
        GRER,
        GFER,
        GEDR,
        GAFR_L,
        GAFR_U,
    } reg;
    int bank;
} pxa2xx_gpio_regs[0x200] = {
    [0 ... 0x1ff] = { GPIO_NONE, 0 },
#define PXA2XX_REG(reg, a0, a1, a2, a3)	\
    [a0] = { reg, 0 }, [a1] = { reg, 1 }, [a2] = { reg, 2 }, [a3] = { reg, 3 }, 

    PXA2XX_REG(GPLR, 0x000, 0x004, 0x008, 0x100)
    PXA2XX_REG(GPSR, 0x018, 0x01c, 0x020, 0x118)
    PXA2XX_REG(GPCR, 0x024, 0x028, 0x02c, 0x124)
    PXA2XX_REG(GPDR, 0x00c, 0x010, 0x014, 0x10c)
    PXA2XX_REG(GRER, 0x030, 0x034, 0x038, 0x130)
    PXA2XX_REG(GFER, 0x03c, 0x040, 0x044, 0x13c)
    PXA2XX_REG(GEDR, 0x048, 0x04c, 0x050, 0x148)
    PXA2XX_REG(GAFR_L, 0x054, 0x05c, 0x064, 0x06c)
    PXA2XX_REG(GAFR_U, 0x058, 0x060, 0x068, 0x070)
};

static void pxa2xx_gpio_irq_update(struct pxa2xx_gpio_info_s *s)
{
    if (s->status[0] & (1 << 0))
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_0, 1);
    else
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_0, 0);

    if (s->status[0] & (1 << 1))
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_1, 1);
    else
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_1, 0);

    if ((s->status[0] & ~3) | s->status[1] | s->status[2] | s->status[3])
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_X, 1);
    else
        pic_set_irq_new(s->pic, PXA2XX_PIC_GPIO_X, 0);
}

/* Bitmap of pins used as standby and sleep wake-up sources.  */
const int pxa2xx_gpio_wake[PXA2XX_GPIO_BANKS] = {
    0x8003fe1b, 0x002001fc, 0xec080000, 0x0012007f,
};

void pxa2xx_gpio_set(struct pxa2xx_gpio_info_s *s, int line, int level)
{
    int bank;
    uint32_t mask;

    if (line >= s->lines) {
        printf("%s: No GPIO pin %i\n", __FUNCTION__, line);
        return;
    }

    bank = line >> 5;
    mask = 1 << (line & 31);

    if (level) {
        s->status[bank] |= s->rising[bank] & mask &
                ~s->ilevel[bank] & ~s->dir[bank];
        s->ilevel[bank] |= mask;
    } else {
        s->status[bank] |= s->falling[bank] & mask &
                s->ilevel[bank] & ~s->dir[bank];
        s->ilevel[bank] &= ~mask;
    }

    if (s->status[bank] & mask)
        pxa2xx_gpio_irq_update(s);

    /* Wake-up GPIOs */
    if (s->cpu_env->halted && (mask & ~s->dir[bank] & pxa2xx_gpio_wake[bank]))
        cpu_interrupt(s->cpu_env, CPU_INTERRUPT_EXITTB);
}

static void pxa2xx_gpio_handler_update(struct pxa2xx_gpio_info_s *s) {
    uint32_t level, diff;
    int i, bit, line;
    for (i = 0; i < PXA2XX_GPIO_BANKS; i ++) {
        level = s->olevel[i] & s->dir[i];

        for (diff = s->prev_level[i] ^ level; diff; diff ^= 1 << bit) {
            bit = ffs(diff) - 1;
            line = bit + 32 * i;
            if (s->handler[line].fn)
                s->handler[line].fn(line, (level >> bit) & 1,
                                s->handler[line].opaque);
        }

        s->prev_level[i] = level;
    }
}

static uint32_t pxa2xx_gpio_read(void *opaque, target_phys_addr_t offset)
{
    struct pxa2xx_gpio_info_s *s = (struct pxa2xx_gpio_info_s *) opaque;
    uint32_t ret;
    int bank;
    offset -= s->base;
    if (offset >= 0x200)
        return 0;

    bank = pxa2xx_gpio_regs[offset].bank;
    switch (pxa2xx_gpio_regs[offset].reg) {
    case GPDR:		/* GPIO Pin-Direction Registers */
        return s->dir[bank];

    case GRER:		/* GPIO Rising-Edge Detect Enable Registers */
        return s->rising[bank];

    case GFER:		/* GPIO Falling-Edge Detect Enable Registers */
        return s->falling[bank];

    case GAFR_L:	/* GPIO Alternate Function Registers */
        return s->gafr[bank * 2];

    case GAFR_U:	/* GPIO Alternate Function Registers */
        return s->gafr[bank * 2 + 1];

    case GPLR:		/* GPIO Pin-Level Registers */
        ret = (s->olevel[bank] & s->dir[bank]) |
                (s->ilevel[bank] & ~s->dir[bank]);
        if (s->read_notify)
            s->read_notify(s->opaque);
        return ret;

    case GEDR:		/* GPIO Edge Detect Status Registers */
        return s->status[bank];

    default:
        cpu_abort(cpu_single_env,
                "%s: Bad offset %x\n", __FUNCTION__, offset);
    }

    return 0;
}

static void pxa2xx_gpio_write(void *opaque,
                target_phys_addr_t offset, uint32_t value)
{
    struct pxa2xx_gpio_info_s *s = (struct pxa2xx_gpio_info_s *) opaque;
    int bank;
    offset -= s->base;
    if (offset >= 0x200)
        return;

    bank = pxa2xx_gpio_regs[offset].bank;
    switch (pxa2xx_gpio_regs[offset].reg) {
    case GPDR:		/* GPIO Pin-Direction Registers */
        s->dir[bank] = value;
        pxa2xx_gpio_handler_update(s);
        break;

    case GPSR:		/* GPIO Pin-Output Set Registers */
        s->olevel[bank] |= value;
        pxa2xx_gpio_handler_update(s);
        break;

    case GPCR:		/* GPIO Pin-Output Clear Registers */
        s->olevel[bank] &= ~value;
        pxa2xx_gpio_handler_update(s);
        break;

    case GRER:		/* GPIO Rising-Edge Detect Enable Registers */
        s->rising[bank] = value;
        break;

    case GFER:		/* GPIO Falling-Edge Detect Enable Registers */
        s->falling[bank] = value;
        break;

    case GAFR_L:	/* GPIO Alternate Function Registers */
        s->gafr[bank * 2] = value;
        break;

    case GAFR_U:	/* GPIO Alternate Function Registers */
        s->gafr[bank * 2 + 1] = value;
        break;

    case GEDR:		/* GPIO Edge Detect Status Registers */
        s->status[bank] &= ~value;
        pxa2xx_gpio_irq_update(s);
        break;

    default:
        cpu_abort(cpu_single_env,
                "%s: Bad offset %x\n", __FUNCTION__, offset);
    }
}

static CPUReadMemoryFunc *pxa2xx_gpio_readfn[] = {
    pxa2xx_gpio_read,
    pxa2xx_gpio_read,
    pxa2xx_gpio_read
};

static CPUWriteMemoryFunc *pxa2xx_gpio_writefn[] = {
    pxa2xx_gpio_write,
    pxa2xx_gpio_write,
    pxa2xx_gpio_write
};

struct pxa2xx_gpio_info_s *pxa2xx_gpio_init(target_phys_addr_t base,
                CPUState *env, void *pic, int lines)
{
    int iomemtype;
    struct pxa2xx_gpio_info_s *s;

    s = (struct pxa2xx_gpio_info_s *)
            qemu_mallocz(sizeof(struct pxa2xx_gpio_info_s));
    memset(s, 0, sizeof(struct pxa2xx_gpio_info_s));
    s->base = base;
    s->pic = pic;
    s->lines = lines;
    s->cpu_env = env;

    iomemtype = cpu_register_io_memory(0, pxa2xx_gpio_readfn,
                    pxa2xx_gpio_writefn, s);
    cpu_register_physical_memory(base, 0x00000fff, iomemtype);

    return s;
}

void pxa2xx_gpio_handler_set(struct pxa2xx_gpio_info_s *s, int line,
                gpio_handler_t handler, void *opaque) {
    if (line >= s->lines) {
        printf("%s: No GPIO pin %i\n", __FUNCTION__, line);
        return;
    }

    s->handler[line].fn = handler;
    s->handler[line].opaque = opaque;
}

/*
 * Registers a callback to notify on GPLR reads.  This normally
 * shouldn't be needed but it is used for the hack on Spitz machines.
 */
void pxa2xx_gpio_read_notifier(struct pxa2xx_gpio_info_s *s,
                void (*handler)(void *opaque), void *opaque) {
    s->read_notify = handler;
    s->opaque = opaque;
}
