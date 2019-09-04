/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2017-2019 by TOPPERS PROJECT Educational Working Group.
 * 
 *  �嵭����Ԥϡ��ʲ���(1)��(4)�ξ������������˸¤ꡤ�ܥ��եȥ���
 *  �����ܥ��եȥ���������Ѥ�����Τ�ޤࡥ�ʲ�Ʊ���ˤ���ѡ�ʣ������
 *  �ѡ������ۡʰʲ������ѤȸƤ֡ˤ��뤳�Ȥ�̵���ǵ������롥
 *  (1) �ܥ��եȥ������򥽡��������ɤη������Ѥ�����ˤϡ��嵭������
 *      ��ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ��꤬�����Τޤޤη��ǥ���
 *      ����������˴ޤޤ�Ƥ��뤳�ȡ�
 *  (2) �ܥ��եȥ������򡤥饤�֥������ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ�����Ǻ����ۤ�����ˤϡ������ۤ�ȼ���ɥ�����ȡ�����
 *      �ԥޥ˥奢��ʤɡˤˡ��嵭�����ɽ�����������Ѿ�浪��Ӳ���
 *      ��̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *  (3) �ܥ��եȥ������򡤵�����Ȥ߹���ʤɡ�¾�Υ��եȥ�������ȯ�˻�
 *      �ѤǤ��ʤ����Ǻ����ۤ�����ˤϡ����Τ����줫�ξ�����������
 *      �ȡ�
 *    (a) �����ۤ�ȼ���ɥ�����ȡ����Ѽԥޥ˥奢��ʤɡˤˡ��嵭����
 *        �ɽ�����������Ѿ�浪��Ӳ�����̵�ݾڵ����Ǻܤ��뤳�ȡ�
 *    (b) �����ۤη��֤��̤�������ˡ�ˤ�äơ�TOPPERS�ץ��������Ȥ�
 *        ��𤹤뤳�ȡ�
 *  (4) �ܥ��եȥ����������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������뤤���ʤ�»
 *      ������⡤�嵭����Ԥ����TOPPERS�ץ��������Ȥ����դ��뤳�ȡ�
 *      �ޤ����ܥ��եȥ������Υ桼���ޤ��ϥ���ɥ桼������Τ����ʤ���
 *      ͳ�˴�Ť����ᤫ��⡤�嵭����Ԥ����TOPPERS�ץ��������Ȥ�
 *      ���դ��뤳�ȡ�
 * 
 *  �ܥ��եȥ������ϡ�̵�ݾڤ��󶡤���Ƥ����ΤǤ��롥�嵭����Ԥ�
 *  ���TOPPERS�ץ��������Ȥϡ��ܥ��եȥ������˴ؤ��ơ�����λ�����Ū
 *  ���Ф���Ŭ������ޤ�ơ������ʤ��ݾڤ�Ԥ�ʤ����ޤ����ܥ��եȥ���
 *  �������Ѥˤ��ľ��Ū�ޤ��ϴ���Ū�������������ʤ�»���˴ؤ��Ƥ⡤��
 *  ����Ǥ�����ʤ���
 * 
 *  @(#) $Id: target_serial.c 2246 2019-01-12 00:16:51Z roi $
 */

/*
 *		���ꥢ��I/O�ǥХ�����SIO�˥ɥ饤�С�HIFIVE1�ѡ�
 */

#include <kernel.h>
#include <t_syslog.h>
#include "target_stddef.h"
#include "target_serial.h"
#include "target_syssvc.h"
#include "/home/suga/risc-v/asp/target/hifive1_gcc/target_clib.h"
#include "/home/suga/risc-v/asp/target/hifive1_gcc/target_syscalls.h"

extern int putchar1(int ch);
/*
 *  SIL�ؿ��Υޥ������
 */
#define sil_orw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) | (b))
#define sil_andw_mem(a, b)		sil_wrw_mem((a), sil_rew_mem(a) & ~(b))

/*
 * �쥸����������
 */
#define INDEX_PORT(x)	((x) - 1)
#define GET_SIOPCB(x)	(&siopcb_table[INDEX_PORT(x)])

/*
 *  ���ꥢ��I/O�ݡ��Ƚ�����֥��å������
 */
typedef struct sio_port_initialization_block {
	uint32_t base;
	INTNO    intno_usart;
	uint32_t iof;
} SIOPINIB;

/*
 *  ���ꥢ��I/O�ݡ��ȴ����֥��å������
 */
struct sio_port_control_block {
	const SIOPINIB  *p_siopinib;  /* ���ꥢ��I/O�ݡ��Ƚ�����֥��å� */
	intptr_t        exinf;        /* ��ĥ���� */
	bool_t          opnflg;       /* �����ץ�Ѥߥե饰 */
	int32_t         rxdata;       /* ����FIFO�ǡ�����¸�ΰ� */
};

/*
 * ���ꥢ��I/O�ݡ��Ƚ�����֥��å�
 */
const SIOPINIB siopinib_table[TNUM_SIOP] = {
	{(uint32_t)TADR_UART0_BASE, (INTNO)IRQ_VECTOR_UART0, IOF0_UART0_MASK},
#if TNUM_SIOP >= 2
	{(uint32_t)TADR_UART1_BASE, (INTNO)IRQ_VECTOR_UART1, IOF0_UART1_MASK}
#endif
};

/*
 *  ���ꥢ��I/O�ݡ��ȴ����֥��å��Υ��ꥢ
 */
SIOPCB	siopcb_table[TNUM_SIOP];

/*
 *  ���ꥢ��I/O�ݡ���ID��������֥��å�����Ф�����Υޥ���
 */
#define INDEX_SIOP(siopid)	((uint_t)((siopid) - 1))
#define get_siopcb(siopid)	(&(siopcb_table[INDEX_SIOP(siopid)]))


void put_hex(char a, int val)
{
	int i, j;
	target_fput_log(a);
	target_fput_log(' ');
	for(i = 28 ; i >= 0 ; i-= 4){
		j = (val >> i) & 0xf;;
		if(j > 9)
			j += ('A'-10);
		else
			j += '0';
		target_fput_log(j);
	}
	target_fput_log('\n');
}

/*
 *  SIO�ɥ饤�Фν����
 */
void
sio_initialize(intptr_t exinf)
{
	SIOPCB	*p_siopcb;
	uint_t	i;

	//putchar1("a");
	/*
	 *  ���ꥢ��I/O�ݡ��ȴ����֥��å��ν����
	 */
	for (p_siopcb = siopcb_table, i = 0; i < TNUM_SIOP; p_siopcb++, i++) {
		p_siopcb->p_siopinib = &(siopinib_table[i]);
		p_siopcb->opnflg = false;
	}
}


/*
 *  ���ꥢ��I/O�ݡ��ȤΥ����ץ�
 */
SIOPCB *
sio_opn_por(ID siopid, intptr_t exinf)
{
	SIOPCB          *p_siopcb;
	const SIOPINIB  *p_siopinib;
	bool_t   opnflg;
	ER       ercd;
	uint32_t base;

	p_siopcb = get_siopcb(siopid);
	p_siopinib = p_siopcb->p_siopinib;

	/*
	 *  �����ץ󤷤��ݡ��Ȥ����뤫��opnflg���ɤ�Ǥ�����
	 */
	opnflg = p_siopcb->opnflg;

	p_siopcb->exinf = exinf;
	base = p_siopinib->base;
	if(base == 0)				/* no uart port */
		goto sio_opn_exit;

	/*
	 *  �ϡ��ɥ������ν����
	 */
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_OUTPUT_VAL), p_siopinib->iof);
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_OUTPUT_EN), p_siopinib->iof);
	sil_andw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_IOF_SEL), p_siopinib->iof);
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_IOF_EN), p_siopinib->iof);

	sil_wrw_mem((uint32_t *)(base+TOFF_UART_DIV), SYS_CLOCK / BPS_SETTING - 1);
	sil_wrw_mem((uint32_t *)(base+TOFF_UART_TXCTRL), UART_TXEN + 0x10000);
	sil_wrw_mem((uint32_t *)(base+TOFF_UART_RXCTRL), UART_RXEN);
	p_siopcb->opnflg = true;

	/*
	 *  ���ꥢ��I/O����ߤΥޥ����������롥
	 */
	if (!opnflg) {
		ercd = ena_int(p_siopinib->intno_usart);
		assert(ercd == E_OK);
	}
	sil_dly_nse(10000);

sio_opn_exit:;
	return(p_siopcb);
}

/*
 *  ���ꥢ��I/O�ݡ��ȤΥ�������
 */
void
sio_cls_por(SIOPCB *p_siopcb)
{
	/*
	 *  ���ꥢ��I/O����ߤ�ޥ������롥
	 */
	if ((p_siopcb->opnflg)) {
		dis_int(p_siopcb->p_siopinib->intno_usart);
	}
	p_siopcb->opnflg = false;
}

/*
 *  SIO�γ���ߥ����ӥ��롼����
 */

Inline bool_t
sio_putintready(SIOPCB* p_siopcb)
{
	uint32_t ip = sil_rew_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IP));
	uint32_t ie = sil_rew_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IE));

	if((ip & UART_IP_TXWM) != 0 && (ie & UART_IP_TXWM) != 0){
		return 1;
	}
	return 0;
}

Inline bool_t
sio_getintready(SIOPCB* p_siopcb)
{
	uint32_t ip = sil_rew_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IP));

	if((ip & UART_IP_RXWM) != 0){
		p_siopcb->rxdata = sil_rew_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_RXFIFO));
		if(p_siopcb->rxdata < 0)
			return 0;
		else
			return 1;
	}
	else
		return 0;
}

Inline bool_t
sio_putready(SIOPCB* p_siopcb)
{
	uint32_t txfifo = sil_rew_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_TXFIFO));

	if((txfifo & UART_TXFIFO_FULL) == 0){
		return 1;
	}
	return 0;
}

void
sio_isr(intptr_t exinf)
{
	SIOPCB          *p_siopcb;

	p_siopcb = get_siopcb(exinf);

	if (sio_getintready(p_siopcb)) {
		sio_irdy_rcv(p_siopcb->exinf);
	}
	if (sio_putintready(p_siopcb)) {
		sio_irdy_snd(p_siopcb->exinf);
	}
}

/*
 *  ���ꥢ��I/O�ݡ��Ȥؤ�ʸ������
 */
bool_t
sio_snd_chr(SIOPCB *p_siopcb, char c)
{
#ifndef DEFAULT_CLOCK
	if(sio_putready(p_siopcb)){
		sil_wrw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_TXFIFO), (uint32_t)c);
		return true;
	}
	return false;
#else
	while(sio_putready(p_siopcb) == 0);
	sil_wrw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_TXFIFO), (uint32_t)c);
	return true;
#endif
}

/*
 *  ���ꥢ��I/O�ݡ��Ȥ����ʸ������
 */
int_t
sio_rcv_chr(SIOPCB *p_siopcb)
{
	int_t c = -1;

	if(p_siopcb->rxdata >= 0){
		c = p_siopcb->rxdata & 0xFF;
		p_siopcb->rxdata = -1;
	}
	return c;
}

/*
 *  ���ꥢ��I/O�ݡ��Ȥ���Υ�����Хå��ε���
 */
void
sio_ena_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		sil_orw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IE), UART_IP_TXWM);
		break;
	case SIO_RDY_RCV:
		sil_orw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IE), UART_IP_RXWM);
		break;
	}
}

/*
 *  ���ꥢ��I/O�ݡ��Ȥ���Υ�����Хå��ζػ�
 */
void
sio_dis_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		sil_andw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IE), UART_IP_TXWM);
		break;
	case SIO_RDY_RCV:
		sil_andw_mem((uint32_t *)(p_siopcb->p_siopinib->base+TOFF_UART_IE), UART_IP_RXWM);
		break;
	}
}

/*
 *  1ʸ�����ϡʥݡ���󥰤Ǥν��ϡ�
 */
void sio_pol_snd_chr(int8_t c, ID siopid)
{
	uint32_t base = siopinib_table[INDEX_PORT(siopid)].base;

	sil_wrw_mem((uint32_t *)(base+TOFF_UART_TXFIFO), (uint32_t)c);
	while(0 != (sil_rew_mem((uint32_t *)(base+TOFF_UART_TXFIFO)) & UART_TXFIFO_FULL));

	/*
	 *  ���Ϥ������˽����ޤ��Ԥ�
	 */
	volatile int n = SystemFrequency/BPS_SETTING;
	while(n--);
}

/*
 *  �������åȤΥ��ꥢ������
 */
void target_uart_init(ID siopid)
{
	const SIOPINIB  *p_siopinib;
	uint32_t base;

	p_siopinib = &siopinib_table[INDEX_PORT(siopid)];
	base = p_siopinib->base;

	/*
	 *  �ϡ��ɥ������ν����
	 */
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_OUTPUT_VAL), p_siopinib->iof);
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_OUTPUT_EN), p_siopinib->iof);
	sil_andw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_IOF_SEL), p_siopinib->iof);
	sil_orw_mem((uint32_t *)(TADR_GPIO_BASE+TOFF_GPIO_IOF_EN), p_siopinib->iof);

	sil_wrw_mem((uint32_t *)(base+TOFF_UART_DIV), SYS_CLOCK / BPS_SETTING - 1);
	sil_wrw_mem((uint32_t *)(base+TOFF_UART_TXCTRL), UART_TXEN);
	sil_wrw_mem((uint32_t *)(base+TOFF_UART_RXCTRL), UART_RXEN);
	sil_dly_nse(10000);
}
