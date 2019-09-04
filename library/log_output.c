/*
 *  TOPPERS Software
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  @(#) $Id: log_output.c 2246 2011-08-26 22:39:15Z ertl-hiro $
 */

/*
 *		�����ƥ�����Υե����ޥåȽ���
 */

#include <t_stddef.h>
#include <t_syslog.h>
#include <log_output.h>
#include "target_syscalls.h"
#include "target_clib.h"

/*
 *  ���ͤ�ʸ������Ѵ�
 */
#define CONVERT_BUFLEN	((sizeof(uintptr_t) * CHAR_BIT + 2) / 3)
										/* uintptr_t���ο��ͤκ���ʸ���� */
static void
convert(uintptr_t val, uint_t radix, const char *radchar,
			uint_t width, bool_t minus, bool_t padzero, void (*putc)(char))
{
	char	buf[CONVERT_BUFLEN];
	uint_t	i, j;

	i = 0U;
	do {
		buf[i++] = radchar[val % radix];
		val /= radix;
	} while (i < CONVERT_BUFLEN && val != 0);

	if (minus && width > 0) {
		width -= 1;
	}
	if (minus && padzero) {
		(*putc)('-');
	}
	for (j = i; j < width; j++) {
		(*putc)(padzero ? '0' : ' ');
	}
	if (minus && !padzero) {
		(*putc)('-');
	}
	while (i > 0U) {
		(*putc)(buf[--i]);
	}
}

/*
 *  ʸ������������
 */
static const char raddec[] = "0123456789";
static const char radhex[] = "0123456789abcdef";
static const char radHEX[] = "0123456789ABCDEF";

void
syslog_printf(const char *format, const intptr_t *p_args, void (*putc)(char))
{
	char		c;
	uint_t		width;
	bool_t		padzero;
	intptr_t	val;
	const char	*str;

	while ((c = *format++) != '\0') {
		if (c != '%') {
			(*putc)(c);
			continue;
		}

		width = 0U;
		padzero = false;
		if ((c = *format++) == '0') {
			padzero = true;
			c = *format++;
		}
		while ('0' <= c && c <= '9') {
			width = width * 10U + c - '0';
			c = *format++;
		}
		if (c == 'l') {
			c = *format++;
		}
		switch (c) {
		case 'd':
			val = (intptr_t)(*p_args++);
			if (val >= 0) {
				convert((uintptr_t) val, 10U, raddec,
										width, false, padzero, putc);
			}
			else {
				convert((uintptr_t)(-val), 10U, raddec,
										width, true, padzero, putc);
			}
			break;
		case 'u':
			val = (intptr_t)(*p_args++);
			convert((uintptr_t) val, 10U, raddec, width, false, padzero, putc);
			break;
		case 'x':
		case 'p':
			val = (intptr_t)(*p_args++);
			convert((uintptr_t) val, 16U, radhex, width, false, padzero, putc);
			break;
		case 'X':
			val = (intptr_t)(*p_args++);
			convert((uintptr_t) val, 16U, radHEX, width, false, padzero, putc);
			break;
		case 'c':
			(*putc)((char)(intptr_t)(*p_args++));
			break;
		case 's':
			str = (const char *)(*p_args++);
			while ((c = *str++) != '\0') {
				(*putc)(c);
			}
			break;
		case '%':
			(*putc)('%');
			break;
		case '\0':
			format--;
			break;
		default:
			break;
		}
	}
}

/*
 *  ��������ν���
 */
void
syslog_print(const SYSLOG *p_syslog, void (*putc)(char))
{
	switch (p_syslog->logtype) {
	case LOG_TYPE_COMMENT:
	    printf_spike("Hello\n");
		syslog_printf((const char *)(p_syslog->loginfo[0]),
								&(p_syslog->loginfo[1]), putc);
		break;
	case LOG_TYPE_ASSERT:
	    printf_spike("Hello\n");
		syslog_printf("%s:%u: Assertion `%s' failed.",
								&(p_syslog->loginfo[0]), putc);
		break;
	default:
		/*
		 *  ¾�μ��̤Υ�������ˤ��б����Ƥ��ʤ���
		 */
		break;
	}
}

/*
 *  ���������Ӽ���å������ν���
 */
void
syslog_lostmsg(uint_t lostlog, void (*putc)(char))
{
	intptr_t	lostinfo[1];

	lostinfo[0] = (intptr_t) lostlog;
	syslog_printf("%d messages are lost.", lostinfo, putc);
	(*putc)('\n');
}
