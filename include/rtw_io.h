/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *
 ******************************************************************************/

#ifndef _RTW_IO_H_
#define _RTW_IO_H_

#include <autoconf.h>
#include <osdep_service.h>
#include <osdep_intf.h>

#include <asm/byteorder.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <asm/atomic.h>

#include <linux/usb.h>
#include <linux/usb/ch9.h>

#define rtw_usb_buffer_alloc(dev, size, mem_flags, dma) usb_alloc_coherent((dev), (size), (mem_flags), (dma))
#define rtw_usb_buffer_free(dev, size, addr, dma) usb_free_coherent((dev), (size), (addr), (dma))

#define NUM_IOREQ		8

#define MAX_PROT_SZ	(64-16)

#define _IOREADY			0
#define _IO_WAIT_COMPLETE   1
#define _IO_WAIT_RSP        2

/*  IO COMMAND TYPE */
#define _IOSZ_MASK_		(0x7F)
#define _IO_WRITE_		BIT(7)
#define _IO_FIXED_		BIT(8)
#define _IO_BURST_		BIT(9)
#define _IO_BYTE_		BIT(10)
#define _IO_HW_			BIT(11)
#define _IO_WORD_		BIT(12)
#define _IO_SYNC_		BIT(13)
#define _IO_CMDMASK_	(0x1F80)


/*
	For prompt mode accessing, caller shall free io_req
	Otherwise, io_handler will free io_req
*/



/*  IO STATUS TYPE */
#define _IO_ERR_		BIT(2)
#define _IO_SUCCESS_	BIT(1)
#define _IO_DONE_		BIT(0)


#define IO_RD32			(_IO_SYNC_ | _IO_WORD_)
#define IO_RD16			(_IO_SYNC_ | _IO_HW_)
#define IO_RD8			(_IO_SYNC_ | _IO_BYTE_)

#define IO_RD32_ASYNC	(_IO_WORD_)
#define IO_RD16_ASYNC	(_IO_HW_)
#define IO_RD8_ASYNC	(_IO_BYTE_)

#define IO_WR32			(_IO_WRITE_ | _IO_SYNC_ | _IO_WORD_)
#define IO_WR16			(_IO_WRITE_ | _IO_SYNC_ | _IO_HW_)
#define IO_WR8			(_IO_WRITE_ | _IO_SYNC_ | _IO_BYTE_)

#define IO_WR32_ASYNC	(_IO_WRITE_ | _IO_WORD_)
#define IO_WR16_ASYNC	(_IO_WRITE_ | _IO_HW_)
#define IO_WR8_ASYNC	(_IO_WRITE_ | _IO_BYTE_)

/*

	Only Sync. burst accessing is provided.

*/

#define IO_WR_BURST(x)		(_IO_WRITE_ | _IO_SYNC_ | _IO_BURST_ | ((x) & _IOSZ_MASK_))
#define IO_RD_BURST(x)		(_IO_SYNC_ | _IO_BURST_ | ((x) & _IOSZ_MASK_))



/* below is for the intf_option bit defition... */

#define _INTF_ASYNC_	BIT(0)	/* support async io */

struct intf_priv;
struct intf_hdl;
struct io_queue;

struct _io_ops
{
		u8 (*_read8)(struct intf_hdl *pintfhdl, u32 addr);
		u16 (*_read16)(struct intf_hdl *pintfhdl, u32 addr);
		u32 (*_read32)(struct intf_hdl *pintfhdl, u32 addr);

		int (*_write8)(struct intf_hdl *pintfhdl, u32 addr, u8 val);
		int (*_write16)(struct intf_hdl *pintfhdl, u32 addr, u16 val);
		int (*_write32)(struct intf_hdl *pintfhdl, u32 addr, u32 val);
		int (*_writeN)(struct intf_hdl *pintfhdl, u32 addr, u32 length, u8 *pdata);

		int (*_write8_async)(struct intf_hdl *pintfhdl, u32 addr, u8 val);
		int (*_write16_async)(struct intf_hdl *pintfhdl, u32 addr, u16 val);
		int (*_write32_async)(struct intf_hdl *pintfhdl, u32 addr, u32 val);

		void (*_read_mem)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
		void (*_write_mem)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);

		void (*_sync_irp_protocol_rw)(struct io_queue *pio_q);

		u32 (*_read_interrupt)(struct intf_hdl *pintfhdl, u32 addr);

		u32 (*_read_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
		u32 (*_write_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);

		u32 (*_write_scsi)(struct intf_hdl *pintfhdl,u32 cnt, u8 *pmem);

		void (*_read_port_cancel)(struct intf_hdl *pintfhdl);
		void (*_write_port_cancel)(struct intf_hdl *pintfhdl);

};

struct io_req {
	struct list_head list;
	u32	addr;
	volatile u32	val;
	u32	command;
	u32	status;
	u8	*pbuf;
	struct  semaphore sema;

	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt);
	u8 *cnxt;
};

struct	intf_hdl {
	struct rtw_adapter *padapter;
	struct dvobj_priv *pintf_dev;/* 	pointer to &(padapter->dvobjpriv); */
	struct _io_ops	io_ops;
};

struct reg_protocol_rd {

#ifdef __LITTLE_ENDIAN

	/* DW1 */
	u32		NumOfTrans:4;
	u32		Reserved1:4;
	u32		Reserved2:24;
	/* DW2 */
	u32		ByteCount:7;
	u32		WriteEnable:1;		/* 0:read, 1:write */
	u32		FixOrContinuous:1;	/* 0:continuous, 1: Fix */
	u32		BurstMode:1;
	u32		Byte1Access:1;
	u32		Byte2Access:1;
	u32		Byte4Access:1;
	u32		Reserved3:3;
	u32		Reserved4:16;
	/* DW3 */
	u32		BusAddress;
	/* DW4 */
#else


/* DW1 */
	u32 Reserved1  :4;
	u32 NumOfTrans :4;

	u32 Reserved2  :24;

	/* DW2 */
	u32 WriteEnable : 1;
	u32 ByteCount :7;


	u32 Reserved3 : 3;
	u32 Byte4Access : 1;

	u32 Byte2Access : 1;
	u32 Byte1Access : 1;
	u32 BurstMode :1 ;
	u32 FixOrContinuous : 1;

	u32 Reserved4 : 16;

	/* DW3 */
	u32		BusAddress;

	/* DW4 */

#endif

};


struct reg_protocol_wt {


#ifdef __LITTLE_ENDIAN

	/* DW1 */
	u32		NumOfTrans:4;
	u32		Reserved1:4;
	u32		Reserved2:24;
	/* DW2 */
	u32		ByteCount:7;
	u32		WriteEnable:1;		/* 0:read, 1:write */
	u32		FixOrContinuous:1;	/* 0:continuous, 1: Fix */
	u32		BurstMode:1;
	u32		Byte1Access:1;
	u32		Byte2Access:1;
	u32		Byte4Access:1;
	u32		Reserved3:3;
	u32		Reserved4:16;
	/* DW3 */
	u32		BusAddress;
	/* DW4 */
	u32		Value;

#else
	/* DW1 */
	u32 Reserved1  :4;
	u32 NumOfTrans :4;

	u32 Reserved2  :24;

	/* DW2 */
	u32 WriteEnable : 1;
	u32 ByteCount :7;

	u32 Reserved3 : 3;
	u32 Byte4Access : 1;

	u32 Byte2Access : 1;
	u32 Byte1Access : 1;
	u32 BurstMode :1 ;
	u32 FixOrContinuous : 1;

	u32 Reserved4 : 16;

	/* DW3 */
	u32		BusAddress;

	/* DW4 */
	u32		Value;

#endif

};



/*
Below is the data structure used by _io_handler

*/

struct io_queue {
	spinlock_t lock;
	struct list_head free_ioreqs;
	struct list_head pending;		/* The io_req list that will be served in the single protocol read/write. */
	struct list_head processing;
	u8	*free_ioreqs_buf; /*  4-byte aligned */
	u8	*pallocated_free_ioreqs_buf;
	struct	intf_hdl	intf;
};

struct io_priv{

	struct rtw_adapter *padapter;

	struct intf_hdl intf;

};

uint ioreq_flush(struct rtw_adapter *adapter, struct io_queue *ioqueue);
void sync_ioreq_enqueue(struct io_req *preq,struct io_queue *ioqueue);
uint sync_ioreq_flush(struct rtw_adapter *adapter, struct io_queue *ioqueue);


uint free_ioreq(struct io_req *preq, struct io_queue *pio_queue);
struct io_req *alloc_ioreq(struct io_queue *pio_q);

uint register_intf_hdl(u8 *dev, struct intf_hdl *pintfhdl);
void unregister_intf_hdl(struct intf_hdl *pintfhdl);

void _rtw_attrib_read(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void _rtw_attrib_write(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);

u8 rtw_read8(struct rtw_adapter *adapter, u32 addr);
u16 rtw_read16(struct rtw_adapter *adapter, u32 addr);
u32 rtw_read32(struct rtw_adapter *adapter, u32 addr);
void rtw_read_mem(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void rtw_read_port(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void rtw_read_port_cancel(struct rtw_adapter *adapter);


int rtw_write8(struct rtw_adapter *adapter, u32 addr, u8 val);
int rtw_write16(struct rtw_adapter *adapter, u32 addr, u16 val);
int rtw_write32(struct rtw_adapter *adapter, u32 addr, u32 val);
int rtw_writeN(struct rtw_adapter *adapter, u32 addr, u32 length, u8 *pdata);

int rtw_write8_async(struct rtw_adapter *adapter, u32 addr, u8 val);
int rtw_write16_async(struct rtw_adapter *adapter, u32 addr, u16 val);
int rtw_write32_async(struct rtw_adapter *adapter, u32 addr, u32 val);

void rtw_write_mem(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
u32 rtw_write_port(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
u32 rtw_write_port_and_wait(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem, int timeout_ms);
void rtw_write_port_cancel(struct rtw_adapter *adapter);

void rtw_write_scsi(struct rtw_adapter *adapter, u32 cnt, u8 *pmem);

/* ioreq */
void ioreq_read8(struct rtw_adapter *adapter, u32 addr, u8 *pval);
void ioreq_read16(struct rtw_adapter *adapter, u32 addr, u16 *pval);
void ioreq_read32(struct rtw_adapter *adapter, u32 addr, u32 *pval);
void ioreq_write8(struct rtw_adapter *adapter, u32 addr, u8 val);
void ioreq_write16(struct rtw_adapter *adapter, u32 addr, u16 val);
void ioreq_write32(struct rtw_adapter *adapter, u32 addr, u32 val);


uint async_read8(struct rtw_adapter *adapter, u32 addr, u8 *pbuff,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
uint async_read16(struct rtw_adapter *adapter, u32 addr,  u8 *pbuff,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
uint async_read32(struct rtw_adapter *adapter, u32 addr,  u8 *pbuff,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);

void async_read_mem(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void async_read_port(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);

void async_write8(struct rtw_adapter *adapter, u32 addr, u8 val,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
void async_write16(struct rtw_adapter *adapter, u32 addr, u16 val,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
void async_write32(struct rtw_adapter *adapter, u32 addr, u32 val,
	void (*_async_io_callback)(struct rtw_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);

void async_write_mem(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void async_write_port(struct rtw_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);

int rtw_init_io_priv(struct rtw_adapter *padapter, void (*set_intf_ops)(struct _io_ops *pops));

uint alloc_io_queue(struct rtw_adapter *adapter);
void free_io_queue(struct rtw_adapter *adapter);
void async_bus_io(struct io_queue *pio_q);
void bus_sync_io(struct io_queue *pio_q);
u32 _ioreq2rwmem(struct io_queue *pio_q);
void dev_power_down(struct rtw_adapter * adapter, u8 bpwrup);

#endif	/* _RTL8711_IO_H_ */
