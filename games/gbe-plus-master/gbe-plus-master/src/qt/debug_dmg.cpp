// GB Enhanced+ Copyright Daniel Baxter 2015
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : debug_dmg.cpp
// Date : November 21, 2015
// Description : DMG/GBC debugging UI
//
// Dialog for DMG/GBC debugging
// Shows MMIO registers, CPU state, instructions, memory

#include "common/util.h"

#include "debug_dmg.h"
#include "main_menu.h"
#include "render.h"

/****** DMG/GBC debug constructor ******/
dmg_debug::dmg_debug(QWidget *parent) : QDialog(parent)
{
	//Set up tabs
	tabs = new QTabWidget(this);

	QDialog* io_regs = new QDialog;
	QDialog* palettes = new QDialog;
	QDialog* memory = new QDialog;
	QDialog* cpu_instr = new QDialog;
	QDialog* vram_obj = new QDialog;
	QDialog* vram_bg = new QDialog;

	tabs->addTab(io_regs, tr("I/O"));
	tabs->addTab(palettes, tr("Palettes"));
	tabs->addTab(memory, tr("Memory"));
	tabs->addTab(cpu_instr, tr("Disassembly"));
	tabs->addTab(vram_obj, tr("OBJ Tiles"));
	tabs->addTab(vram_bg, tr("BG Tiles"));

	//LCDC
	QWidget* lcdc_set = new QWidget(io_regs);
	QLabel* lcdc_label = new QLabel("0xFF40 - LCDC:", lcdc_set);
	mmio_lcdc = new QLineEdit(lcdc_set);
	mmio_lcdc->setMaximumWidth(64);
	mmio_lcdc->setReadOnly(true);

	QHBoxLayout* lcdc_layout = new QHBoxLayout;
	lcdc_layout->addWidget(lcdc_label, 0, Qt::AlignLeft);
	lcdc_layout->addWidget(mmio_lcdc, 0, Qt::AlignRight);
	lcdc_set->setLayout(lcdc_layout);

	//STAT
	QWidget* stat_set = new QWidget(io_regs);
	QLabel* stat_label = new QLabel("0xFF41 - STAT:", stat_set);
	mmio_stat = new QLineEdit(stat_set);
	mmio_stat->setMaximumWidth(64);
	mmio_stat->setReadOnly(true);

	QHBoxLayout* stat_layout = new QHBoxLayout;
	stat_layout->addWidget(stat_label, 0, Qt::AlignLeft);
	stat_layout->addWidget(mmio_stat, 0, Qt::AlignRight);
	stat_set->setLayout(stat_layout);

	//SX
	QWidget* sx_set = new QWidget(io_regs);
	QLabel* sx_label = new QLabel("0xFF43 - SX:", sx_set);
	mmio_sx = new QLineEdit(sx_set);
	mmio_sx->setMaximumWidth(64);
	mmio_sx->setReadOnly(true);

	QHBoxLayout* sx_layout = new QHBoxLayout;
	sx_layout->addWidget(sx_label, 0, Qt::AlignLeft);
	sx_layout->addWidget(mmio_sx, 0, Qt::AlignRight);
	sx_set->setLayout(sx_layout);

	//SY
	QWidget* sy_set = new QWidget(io_regs);
	QLabel* sy_label = new QLabel("0xFF42 - SY:", sy_set);
	mmio_sy = new QLineEdit(sy_set);
	mmio_sy->setMaximumWidth(64);
	mmio_sy->setReadOnly(true);

	QHBoxLayout* sy_layout = new QHBoxLayout;
	sy_layout->addWidget(sy_label, 0, Qt::AlignLeft);
	sy_layout->addWidget(mmio_sy, 0, Qt::AlignRight);
	sy_set->setLayout(sy_layout);

	//LY
	QWidget* ly_set = new QWidget(io_regs);
	QLabel* ly_label = new QLabel("0xFF44 - LY:", ly_set);
	mmio_ly = new QLineEdit(ly_set);
	mmio_ly->setMaximumWidth(64);
	mmio_ly->setReadOnly(true);

	QHBoxLayout* ly_layout = new QHBoxLayout;
	ly_layout->addWidget(ly_label, 0, Qt::AlignLeft);
	ly_layout->addWidget(mmio_ly, 0, Qt::AlignRight);
	ly_set->setLayout(ly_layout);

	//LYC
	QWidget* lyc_set = new QWidget(io_regs);
	QLabel* lyc_label = new QLabel("0xFF45 - LYC:", lyc_set);
	mmio_lyc = new QLineEdit(lyc_set);
	mmio_lyc->setMaximumWidth(64);
	mmio_lyc->setReadOnly(true);

	QHBoxLayout* lyc_layout = new QHBoxLayout;
	lyc_layout->addWidget(lyc_label, 0, Qt::AlignLeft);
	lyc_layout->addWidget(mmio_lyc, 0, Qt::AlignRight);
	lyc_set->setLayout(lyc_layout);

	//DMA
	QWidget* dma_set = new QWidget(io_regs);
	QLabel* dma_label = new QLabel("0xFF46 - DMA:", dma_set);
	mmio_dma = new QLineEdit(dma_set);
	mmio_dma->setMaximumWidth(64);
	mmio_dma->setReadOnly(true);

	QHBoxLayout* dma_layout = new QHBoxLayout;
	dma_layout->addWidget(dma_label, 0, Qt::AlignLeft);
	dma_layout->addWidget(mmio_dma, 0, Qt::AlignRight);
	dma_set->setLayout(dma_layout);

	//BGP
	QWidget* bgp_set = new QWidget(io_regs);
	QLabel* bgp_label = new QLabel("0xFF47 - BGP:", bgp_set);
	mmio_bgp = new QLineEdit(bgp_set);
	mmio_bgp->setMaximumWidth(64);
	mmio_bgp->setReadOnly(true);

	QHBoxLayout* bgp_layout = new QHBoxLayout;
	bgp_layout->addWidget(bgp_label, 0, Qt::AlignLeft);
	bgp_layout->addWidget(mmio_bgp, 0, Qt::AlignRight);
	bgp_set->setLayout(bgp_layout);

	//OBP0
	QWidget* obp0_set = new QWidget(io_regs);
	QLabel* obp0_label = new QLabel("0xFF48 - OBP0:", obp0_set);
	mmio_obp0 = new QLineEdit(obp0_set);
	mmio_obp0->setMaximumWidth(64);
	mmio_obp0->setReadOnly(true);

	QHBoxLayout* obp0_layout = new QHBoxLayout;
	obp0_layout->addWidget(obp0_label, 0, Qt::AlignLeft);
	obp0_layout->addWidget(mmio_obp0, 0, Qt::AlignRight);
	obp0_set->setLayout(obp0_layout);

	//OBP1
	QWidget* obp1_set = new QWidget(io_regs);
	QLabel* obp1_label = new QLabel("0xFF49 - OBP1:", obp1_set);
	mmio_obp1 = new QLineEdit(obp1_set);
	mmio_obp1->setMaximumWidth(64);
	mmio_obp1->setReadOnly(true);

	QHBoxLayout* obp1_layout = new QHBoxLayout;
	obp1_layout->addWidget(obp1_label, 0, Qt::AlignLeft);
	obp1_layout->addWidget(mmio_obp1, 0, Qt::AlignRight);
	obp1_set->setLayout(obp1_layout);

	//WY
	QWidget* wy_set = new QWidget(io_regs);
	QLabel* wy_label = new QLabel("0xFF4A - WY:", wy_set);
	mmio_wy = new QLineEdit(wy_set);
	mmio_wy->setMaximumWidth(64);
	mmio_wy->setReadOnly(true);

	QHBoxLayout* wy_layout = new QHBoxLayout;
	wy_layout->addWidget(wy_label, 0, Qt::AlignLeft);
	wy_layout->addWidget(mmio_wy, 0, Qt::AlignRight);
	wy_set->setLayout(wy_layout);

	//WX
	QWidget* wx_set = new QWidget(io_regs);
	QLabel* wx_label = new QLabel("0xFF4B - WX:", wx_set);
	mmio_wx = new QLineEdit(wx_set);
	mmio_wx->setMaximumWidth(64);
	mmio_wx->setReadOnly(true);

	QHBoxLayout* wx_layout = new QHBoxLayout;
	wx_layout->addWidget(wx_label, 0, Qt::AlignLeft);
	wx_layout->addWidget(mmio_wx, 0, Qt::AlignRight);
	wx_set->setLayout(wx_layout);

	//NR10
	QWidget* nr10_set = new QWidget(io_regs);
	QLabel* nr10_label = new QLabel("0xFF10 - NR10:", nr10_set);
	mmio_nr10 = new QLineEdit(nr10_set);
	mmio_nr10->setMaximumWidth(64);
	mmio_nr10->setReadOnly(true);

	QHBoxLayout* nr10_layout = new QHBoxLayout;
	nr10_layout->addWidget(nr10_label, 0, Qt::AlignLeft);
	nr10_layout->addWidget(mmio_nr10, 0, Qt::AlignRight);
	nr10_set->setLayout(nr10_layout);

	//NR11
	QWidget* nr11_set = new QWidget(io_regs);
	QLabel* nr11_label = new QLabel("0xFF11 - NR11:", nr11_set);
	mmio_nr11 = new QLineEdit(nr11_set);
	mmio_nr11->setMaximumWidth(64);
	mmio_nr11->setReadOnly(true);

	QHBoxLayout* nr11_layout = new QHBoxLayout;
	nr11_layout->addWidget(nr11_label, 0, Qt::AlignLeft);
	nr11_layout->addWidget(mmio_nr11, 0, Qt::AlignRight);
	nr11_set->setLayout(nr11_layout);

	//NR12
	QWidget* nr12_set = new QWidget(io_regs);
	QLabel* nr12_label = new QLabel("0xFF12 - NR12:", nr12_set);
	mmio_nr12 = new QLineEdit(nr12_set);
	mmio_nr12->setMaximumWidth(64);
	mmio_nr12->setReadOnly(true);

	QHBoxLayout* nr12_layout = new QHBoxLayout;
	nr12_layout->addWidget(nr12_label, 0, Qt::AlignLeft);
	nr12_layout->addWidget(mmio_nr12, 0, Qt::AlignRight);
	nr12_set->setLayout(nr12_layout);

	//NR13
	QWidget* nr13_set = new QWidget(io_regs);
	QLabel* nr13_label = new QLabel("0xFF13 - NR13:", nr13_set);
	mmio_nr13 = new QLineEdit(nr13_set);
	mmio_nr13->setMaximumWidth(64);
	mmio_nr13->setReadOnly(true);

	QHBoxLayout* nr13_layout = new QHBoxLayout;
	nr13_layout->addWidget(nr13_label, 0, Qt::AlignLeft);
	nr13_layout->addWidget(mmio_nr13, 0, Qt::AlignRight);
	nr13_set->setLayout(nr13_layout);

	//NR14
	QWidget* nr14_set = new QWidget(io_regs);
	QLabel* nr14_label = new QLabel("0xFF14 - NR14:", nr14_set);
	mmio_nr14 = new QLineEdit(nr14_set);
	mmio_nr14->setMaximumWidth(64);
	mmio_nr14->setReadOnly(true);

	QHBoxLayout* nr14_layout = new QHBoxLayout;
	nr14_layout->addWidget(nr14_label, 0, Qt::AlignLeft);
	nr14_layout->addWidget(mmio_nr14, 0, Qt::AlignRight);
	nr14_set->setLayout(nr14_layout);

	//NR21
	QWidget* nr21_set = new QWidget(io_regs);
	QLabel* nr21_label = new QLabel("0xFF16 - NR21:", nr21_set);
	mmio_nr21 = new QLineEdit(nr21_set);
	mmio_nr21->setMaximumWidth(64);
	mmio_nr21->setReadOnly(true);

	QHBoxLayout* nr21_layout = new QHBoxLayout;
	nr21_layout->addWidget(nr21_label, 0, Qt::AlignLeft);
	nr21_layout->addWidget(mmio_nr21, 0, Qt::AlignRight);
	nr21_set->setLayout(nr21_layout);

	//NR22
	QWidget* nr22_set = new QWidget(io_regs);
	QLabel* nr22_label = new QLabel("0xFF17 - NR22:", nr22_set);
	mmio_nr22 = new QLineEdit(nr22_set);
	mmio_nr22->setMaximumWidth(64);
	mmio_nr22->setReadOnly(true);

	QHBoxLayout* nr22_layout = new QHBoxLayout;
	nr22_layout->addWidget(nr22_label, 0, Qt::AlignLeft);
	nr22_layout->addWidget(mmio_nr22, 0, Qt::AlignRight);
	nr22_set->setLayout(nr22_layout);

	//NR23
	QWidget* nr23_set = new QWidget(io_regs);
	QLabel* nr23_label = new QLabel("0xFF18 - NR23:", nr23_set);
	mmio_nr23 = new QLineEdit(nr23_set);
	mmio_nr23->setMaximumWidth(64);
	mmio_nr23->setReadOnly(true);

	QHBoxLayout* nr23_layout = new QHBoxLayout;
	nr23_layout->addWidget(nr23_label, 0, Qt::AlignLeft);
	nr23_layout->addWidget(mmio_nr23, 0, Qt::AlignRight);
	nr23_set->setLayout(nr23_layout);

	//NR24
	QWidget* nr24_set = new QWidget(io_regs);
	QLabel* nr24_label = new QLabel("0xFF19 - NR24:", nr24_set);
	mmio_nr24 = new QLineEdit(nr24_set);
	mmio_nr24->setMaximumWidth(64);
	mmio_nr24->setReadOnly(true);

	QHBoxLayout* nr24_layout = new QHBoxLayout;
	nr24_layout->addWidget(nr24_label, 0, Qt::AlignLeft);
	nr24_layout->addWidget(mmio_nr24, 0, Qt::AlignRight);
	nr24_set->setLayout(nr24_layout);

	//NR30
	QWidget* nr30_set = new QWidget(io_regs);
	QLabel* nr30_label = new QLabel("0xFF1A - NR30:", nr30_set);
	mmio_nr30 = new QLineEdit(nr30_set);
	mmio_nr30->setMaximumWidth(64);
	mmio_nr30->setReadOnly(true);

	QHBoxLayout* nr30_layout = new QHBoxLayout;
	nr30_layout->addWidget(nr30_label, 0, Qt::AlignLeft);
	nr30_layout->addWidget(mmio_nr30, 0, Qt::AlignRight);
	nr30_set->setLayout(nr30_layout);

	//NR31
	QWidget* nr31_set = new QWidget(io_regs);
	QLabel* nr31_label = new QLabel("0xFF1B - NR31:", nr31_set);
	mmio_nr31 = new QLineEdit(nr31_set);
	mmio_nr31->setMaximumWidth(64);
	mmio_nr31->setReadOnly(true);

	QHBoxLayout* nr31_layout = new QHBoxLayout;
	nr31_layout->addWidget(nr31_label, 0, Qt::AlignLeft);
	nr31_layout->addWidget(mmio_nr31, 0, Qt::AlignRight);
	nr31_set->setLayout(nr31_layout);

	//NR32
	QWidget* nr32_set = new QWidget(io_regs);
	QLabel* nr32_label = new QLabel("0xFF1C - NR32:", nr32_set);
	mmio_nr32 = new QLineEdit(nr32_set);
	mmio_nr32->setMaximumWidth(64);
	mmio_nr32->setReadOnly(true);

	QHBoxLayout* nr32_layout = new QHBoxLayout;
	nr32_layout->addWidget(nr32_label, 0, Qt::AlignLeft);
	nr32_layout->addWidget(mmio_nr32, 0, Qt::AlignRight);
	nr32_set->setLayout(nr32_layout);

	//NR33
	QWidget* nr33_set = new QWidget(io_regs);
	QLabel* nr33_label = new QLabel("0xFF1D - NR33:", nr33_set);
	mmio_nr33 = new QLineEdit(nr33_set);
	mmio_nr33->setMaximumWidth(64);
	mmio_nr33->setReadOnly(true);

	QHBoxLayout* nr33_layout = new QHBoxLayout;
	nr33_layout->addWidget(nr33_label, 0, Qt::AlignLeft);
	nr33_layout->addWidget(mmio_nr33, 0, Qt::AlignRight);
	nr33_set->setLayout(nr33_layout);

	//NR34
	QWidget* nr34_set = new QWidget(io_regs);
	QLabel* nr34_label = new QLabel("0xFF1E - NR34:", nr34_set);
	mmio_nr34 = new QLineEdit(nr34_set);
	mmio_nr34->setMaximumWidth(64);
	mmio_nr34->setReadOnly(true);

	QHBoxLayout* nr34_layout = new QHBoxLayout;
	nr34_layout->addWidget(nr34_label, 0, Qt::AlignLeft);
	nr34_layout->addWidget(mmio_nr34, 0, Qt::AlignRight);
	nr34_set->setLayout(nr34_layout);

	//NR41
	QWidget* nr41_set = new QWidget(io_regs);
	QLabel* nr41_label = new QLabel("0xFF20 - NR41:", nr41_set);
	mmio_nr41 = new QLineEdit(nr41_set);
	mmio_nr41->setMaximumWidth(64);
	mmio_nr41->setReadOnly(true);

	QHBoxLayout* nr41_layout = new QHBoxLayout;
	nr41_layout->addWidget(nr41_label, 0, Qt::AlignLeft);
	nr41_layout->addWidget(mmio_nr41, 0, Qt::AlignRight);
	nr41_set->setLayout(nr41_layout);

	//NR42
	QWidget* nr42_set = new QWidget(io_regs);
	QLabel* nr42_label = new QLabel("0xFF21 - NR42:", nr42_set);
	mmio_nr42 = new QLineEdit(nr42_set);
	mmio_nr42->setMaximumWidth(64);
	mmio_nr42->setReadOnly(true);

	QHBoxLayout* nr42_layout = new QHBoxLayout;
	nr42_layout->addWidget(nr42_label, 0, Qt::AlignLeft);
	nr42_layout->addWidget(mmio_nr42, 0, Qt::AlignRight);
	nr42_set->setLayout(nr42_layout);

	//NR43
	QWidget* nr43_set = new QWidget(io_regs);
	QLabel* nr43_label = new QLabel("0xFF22 - NR43:", nr43_set);
	mmio_nr43 = new QLineEdit(nr43_set);
	mmio_nr43->setMaximumWidth(64);
	mmio_nr43->setReadOnly(true);

	QHBoxLayout* nr43_layout = new QHBoxLayout;
	nr43_layout->addWidget(nr43_label, 0, Qt::AlignLeft);
	nr43_layout->addWidget(mmio_nr43, 0, Qt::AlignRight);
	nr43_set->setLayout(nr43_layout);

	//NR44
	QWidget* nr44_set = new QWidget(io_regs);
	QLabel* nr44_label = new QLabel("0xFF23 - NR44:", nr44_set);
	mmio_nr44 = new QLineEdit(nr44_set);
	mmio_nr44->setMaximumWidth(64);
	mmio_nr44->setReadOnly(true);

	QHBoxLayout* nr44_layout = new QHBoxLayout;
	nr44_layout->addWidget(nr44_label, 0, Qt::AlignLeft);
	nr44_layout->addWidget(mmio_nr44, 0, Qt::AlignRight);
	nr44_set->setLayout(nr44_layout);

	//NR50
	QWidget* nr50_set = new QWidget(io_regs);
	QLabel* nr50_label = new QLabel("0xFF24 - NR50:", nr50_set);
	mmio_nr50 = new QLineEdit(nr50_set);
	mmio_nr50->setMaximumWidth(64);
	mmio_nr50->setReadOnly(true);

	QHBoxLayout* nr50_layout = new QHBoxLayout;
	nr50_layout->addWidget(nr50_label, 0, Qt::AlignLeft);
	nr50_layout->addWidget(mmio_nr50, 0, Qt::AlignRight);
	nr50_set->setLayout(nr50_layout);

	//NR51
	QWidget* nr51_set = new QWidget(io_regs);
	QLabel* nr51_label = new QLabel("0xFF25 - NR51:", nr51_set);
	mmio_nr51 = new QLineEdit(nr51_set);
	mmio_nr51->setMaximumWidth(64);
	mmio_nr51->setReadOnly(true);

	QHBoxLayout* nr51_layout = new QHBoxLayout;
	nr51_layout->addWidget(nr51_label, 0, Qt::AlignLeft);
	nr51_layout->addWidget(mmio_nr51, 0, Qt::AlignRight);
	nr51_set->setLayout(nr51_layout);

	//NR52
	QWidget* nr52_set = new QWidget(io_regs);
	QLabel* nr52_label = new QLabel("0xFF26 - NR52:", nr52_set);
	mmio_nr52 = new QLineEdit(nr52_set);
	mmio_nr52->setMaximumWidth(64);
	mmio_nr52->setReadOnly(true);

	QHBoxLayout* nr52_layout = new QHBoxLayout;
	nr52_layout->addWidget(nr52_label, 0, Qt::AlignLeft);
	nr52_layout->addWidget(mmio_nr52, 0, Qt::AlignRight);
	nr52_set->setLayout(nr52_layout);

	//KEY1
	QWidget* key1_set = new QWidget(io_regs);
	QLabel* key1_label = new QLabel("0xFF4D - KEY1:", key1_set);
	mmio_key1 = new QLineEdit(key1_set);
	mmio_key1->setMaximumWidth(64);
	mmio_key1->setReadOnly(true);

	QHBoxLayout* key1_layout = new QHBoxLayout;
	key1_layout->addWidget(key1_label, 0, Qt::AlignLeft);
	key1_layout->addWidget(mmio_key1, 0, Qt::AlignRight);
	key1_set->setLayout(key1_layout);

	//HDMA1
	QWidget* hdma1_set = new QWidget(io_regs);
	QLabel* hdma1_label = new QLabel("0xFF51 - HDMA1:", hdma1_set);
	mmio_hdma1 = new QLineEdit(hdma1_set);
	mmio_hdma1->setMaximumWidth(64);
	mmio_hdma1->setReadOnly(true);

	QHBoxLayout* hdma1_layout = new QHBoxLayout;
	hdma1_layout->addWidget(hdma1_label, 0, Qt::AlignLeft);
	hdma1_layout->addWidget(mmio_hdma1, 0, Qt::AlignRight);
	hdma1_set->setLayout(hdma1_layout);

	//HDMA2
	QWidget* hdma2_set = new QWidget(io_regs);
	QLabel* hdma2_label = new QLabel("0xFF52 - HDMA2:", hdma2_set);
	mmio_hdma2 = new QLineEdit(hdma2_set);
	mmio_hdma2->setMaximumWidth(64);
	mmio_hdma2->setReadOnly(true);

	QHBoxLayout* hdma2_layout = new QHBoxLayout;
	hdma2_layout->addWidget(hdma2_label, 0, Qt::AlignLeft);
	hdma2_layout->addWidget(mmio_hdma2, 0, Qt::AlignRight);
	hdma2_set->setLayout(hdma2_layout);

	//HDMA3
	QWidget* hdma3_set = new QWidget(io_regs);
	QLabel* hdma3_label = new QLabel("0xFF53 - HDMA3:", hdma3_set);
	mmio_hdma3 = new QLineEdit(hdma3_set);
	mmio_hdma3->setMaximumWidth(64);
	mmio_hdma3->setReadOnly(true);

	QHBoxLayout* hdma3_layout = new QHBoxLayout;
	hdma3_layout->addWidget(hdma3_label, 0, Qt::AlignLeft);
	hdma3_layout->addWidget(mmio_hdma3, 0, Qt::AlignRight);
	hdma3_set->setLayout(hdma3_layout);

	//HDMA4
	QWidget* hdma4_set = new QWidget(io_regs);
	QLabel* hdma4_label = new QLabel("0xFF54 - HDMA4:", hdma4_set);
	mmio_hdma4 = new QLineEdit(hdma4_set);
	mmio_hdma4->setMaximumWidth(64);
	mmio_hdma4->setReadOnly(true);

	QHBoxLayout* hdma4_layout = new QHBoxLayout;
	hdma4_layout->addWidget(hdma4_label, 0, Qt::AlignLeft);
	hdma4_layout->addWidget(mmio_hdma4, 0, Qt::AlignRight);
	hdma4_set->setLayout(hdma4_layout);

	//HDMA5
	QWidget* hdma5_set = new QWidget(io_regs);
	QLabel* hdma5_label = new QLabel("0xFF55 - HDMA5:", hdma5_set);
	mmio_hdma5 = new QLineEdit(hdma5_set);
	mmio_hdma5->setMaximumWidth(64);
	mmio_hdma5->setReadOnly(true);

	QHBoxLayout* hdma5_layout = new QHBoxLayout;
	hdma5_layout->addWidget(hdma5_label, 0, Qt::AlignLeft);
	hdma5_layout->addWidget(mmio_hdma5, 0, Qt::AlignRight);
	hdma5_set->setLayout(hdma5_layout);

	//RP
	QWidget* rp_set = new QWidget(io_regs);
	QLabel* rp_label = new QLabel("0xFF56 - RP", rp_set);
	mmio_rp = new QLineEdit(rp_set);
	mmio_rp->setMaximumWidth(64);
	mmio_rp->setReadOnly(true);

	//VBK
	QWidget* vbk_set = new QWidget(io_regs);
	QLabel* vbk_label = new QLabel("0xFF4F - VBK:", vbk_set);
	mmio_vbk = new QLineEdit(vbk_set);
	mmio_vbk->setMaximumWidth(64);
	mmio_vbk->setReadOnly(true);

	QHBoxLayout* vbk_layout = new QHBoxLayout;
	vbk_layout->addWidget(vbk_label, 0, Qt::AlignLeft);
	vbk_layout->addWidget(mmio_vbk, 0, Qt::AlignRight);
	vbk_set->setLayout(vbk_layout);

	QHBoxLayout* rp_layout = new QHBoxLayout;
	rp_layout->addWidget(rp_label, 0, Qt::AlignLeft);
	rp_layout->addWidget(mmio_rp, 0, Qt::AlignRight);
	rp_set->setLayout(rp_layout);

	//BCPS
	QWidget* bcps_set = new QWidget(io_regs);
	QLabel* bcps_label = new QLabel("0xFF68 - BCPS:", bcps_set);
	mmio_bcps = new QLineEdit(bcps_set);
	mmio_bcps->setMaximumWidth(64);
	mmio_bcps->setReadOnly(true);

	QHBoxLayout* bcps_layout = new QHBoxLayout;
	bcps_layout->addWidget(bcps_label, 0, Qt::AlignLeft);
	bcps_layout->addWidget(mmio_bcps, 0, Qt::AlignRight);
	bcps_set->setLayout(bcps_layout);

	//BCPD
	QWidget* bcpd_set = new QWidget(io_regs);
	QLabel* bcpd_label = new QLabel("0xFF69 - BCPD:", bcpd_set);
	mmio_bcpd = new QLineEdit(bcpd_set);
	mmio_bcpd->setMaximumWidth(64);
	mmio_bcpd->setReadOnly(true);

	QHBoxLayout* bcpd_layout = new QHBoxLayout;
	bcpd_layout->addWidget(bcpd_label, 0, Qt::AlignLeft);
	bcpd_layout->addWidget(mmio_bcpd, 0, Qt::AlignRight);
	bcpd_set->setLayout(bcpd_layout);

	//OCPS
	QWidget* ocps_set = new QWidget(io_regs);
	QLabel* ocps_label = new QLabel("0xFF6A - OCPS:", ocps_set);
	mmio_ocps = new QLineEdit(ocps_set);
	mmio_ocps->setMaximumWidth(64);
	mmio_ocps->setReadOnly(true);

	QHBoxLayout* ocps_layout = new QHBoxLayout;
	ocps_layout->addWidget(ocps_label, 0, Qt::AlignLeft);
	ocps_layout->addWidget(mmio_ocps, 0, Qt::AlignRight);
	ocps_set->setLayout(ocps_layout);

	//OCPD
	QWidget* ocpd_set = new QWidget(io_regs);
	QLabel* ocpd_label = new QLabel("0xFF6B - OCPD:", ocpd_set);
	mmio_ocpd = new QLineEdit(ocpd_set);
	mmio_ocpd->setMaximumWidth(64);
	mmio_ocpd->setReadOnly(true);

	QHBoxLayout* ocpd_layout = new QHBoxLayout;
	ocpd_layout->addWidget(ocpd_label, 0, Qt::AlignLeft);
	ocpd_layout->addWidget(mmio_ocpd, 0, Qt::AlignRight);
	ocpd_set->setLayout(ocpd_layout);

	//SVBK
	QWidget* svbk_set = new QWidget(io_regs);
	QLabel* svbk_label = new QLabel("0xFF70 - SVBK:", svbk_set);
	mmio_svbk = new QLineEdit(svbk_set);
	mmio_svbk->setMaximumWidth(64);
	mmio_svbk->setReadOnly(true);

	QHBoxLayout* svbk_layout = new QHBoxLayout;
	svbk_layout->addWidget(svbk_label, 0, Qt::AlignLeft);
	svbk_layout->addWidget(mmio_svbk, 0, Qt::AlignRight);
	svbk_set->setLayout(svbk_layout);

	//P1
	QWidget* p1_set = new QWidget(io_regs);
	QLabel* p1_label = new QLabel("0xFF00 - P1:", p1_set);
	mmio_p1 = new QLineEdit(p1_set);
	mmio_p1->setMaximumWidth(64);
	mmio_p1->setReadOnly(true);

	QHBoxLayout* p1_layout = new QHBoxLayout;
	p1_layout->addWidget(p1_label, 0, Qt::AlignLeft);
	p1_layout->addWidget(mmio_p1, 0, Qt::AlignRight);
	p1_set->setLayout(p1_layout);

	//SB
	QWidget* sb_set = new QWidget(io_regs);
	QLabel* sb_label = new QLabel("0xFF01 - SB:", sb_set);
	mmio_sb = new QLineEdit(sb_set);
	mmio_sb->setMaximumWidth(64);
	mmio_sb->setReadOnly(true);

	QHBoxLayout* sb_layout = new QHBoxLayout;
	sb_layout->addWidget(sb_label, 0, Qt::AlignLeft);
	sb_layout->addWidget(mmio_sb, 0, Qt::AlignRight);
	sb_set->setLayout(sb_layout);

	//SC
	QWidget* sc_set = new QWidget(io_regs);
	QLabel* sc_label = new QLabel("0xFF02 - SC:", sc_set);
	mmio_sc = new QLineEdit(sc_set);
	mmio_sc->setMaximumWidth(64);
	mmio_sc->setReadOnly(true);

	QHBoxLayout* sc_layout = new QHBoxLayout;
	sc_layout->addWidget(sc_label, 0, Qt::AlignLeft);
	sc_layout->addWidget(mmio_sc, 0, Qt::AlignRight);
	sc_set->setLayout(sc_layout);

	//DIV
	QWidget* div_set = new QWidget(io_regs);
	QLabel* div_label = new QLabel("0xFF04 - DIV:", div_set);
	mmio_div = new QLineEdit(div_set);
	mmio_div->setMaximumWidth(64);
	mmio_div->setReadOnly(true);

	QHBoxLayout* div_layout = new QHBoxLayout;
	div_layout->addWidget(div_label, 0, Qt::AlignLeft);
	div_layout->addWidget(mmio_div, 0, Qt::AlignRight);
	div_set->setLayout(div_layout);

	//TIMA
	QWidget* tima_set = new QWidget(io_regs);
	QLabel* tima_label = new QLabel("0xFF05 - TIMA:", tima_set);
	mmio_tima = new QLineEdit(tima_set);
	mmio_tima->setMaximumWidth(64);
	mmio_tima->setReadOnly(true);

	QHBoxLayout* tima_layout = new QHBoxLayout;
	tima_layout->addWidget(tima_label, 0, Qt::AlignLeft);
	tima_layout->addWidget(mmio_tima, 0, Qt::AlignRight);
	tima_set->setLayout(tima_layout);

	//TMA
	QWidget* tma_set = new QWidget(io_regs);
	QLabel* tma_label = new QLabel("0xFF06 - TMA:", tma_set);
	mmio_tma = new QLineEdit(tma_set);
	mmio_tma->setMaximumWidth(64);
	mmio_tma->setReadOnly(true);

	QHBoxLayout* tma_layout = new QHBoxLayout;
	tma_layout->addWidget(tma_label, 0, Qt::AlignLeft);
	tma_layout->addWidget(mmio_tma, 0, Qt::AlignRight);
	tma_set->setLayout(tma_layout);

	//TAC
	QWidget* tac_set = new QWidget(io_regs);
	QLabel* tac_label = new QLabel("0xFF07 - TAC:", tac_set);
	mmio_tac = new QLineEdit(tac_set);
	mmio_tac->setMaximumWidth(64);
	mmio_tac->setReadOnly(true);

	QHBoxLayout* tac_layout = new QHBoxLayout;
	tac_layout->addWidget(tac_label, 0, Qt::AlignLeft);
	tac_layout->addWidget(mmio_tac, 0, Qt::AlignRight);
	tac_set->setLayout(tac_layout);

	//IE
	QWidget* ie_set = new QWidget(io_regs);
	QLabel* ie_label = new QLabel("0xFF0F - IE:", ie_set);
	mmio_ie = new QLineEdit(ie_set);
	mmio_ie->setMaximumWidth(64);
	mmio_ie->setReadOnly(true);

	QHBoxLayout* ie_layout = new QHBoxLayout;
	ie_layout->addWidget(ie_label, 0, Qt::AlignLeft);
	ie_layout->addWidget(mmio_ie, 0, Qt::AlignRight);
	ie_set->setLayout(ie_layout);

	//IF
	QWidget* if_set = new QWidget(io_regs);
	QLabel* if_label = new QLabel("0xFFFF - IF:", if_set);
	mmio_if = new QLineEdit(if_set);
	mmio_if->setMaximumWidth(64);
	mmio_if->setReadOnly(true);

	QHBoxLayout* if_layout = new QHBoxLayout;
	if_layout->addWidget(if_label, 0, Qt::AlignLeft);
	if_layout->addWidget(mmio_if, 0, Qt::AlignRight);
	if_set->setLayout(if_layout);

	//MMIO tab layout
	QGridLayout* io_layout = new QGridLayout;
	io_layout->setVerticalSpacing(0);
	io_layout->setHorizontalSpacing(0);
	io_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	io_layout->addWidget(lcdc_set, 0, 0, 1, 1);
	io_layout->addWidget(stat_set, 1, 0, 1, 1);
	io_layout->addWidget(sy_set, 2, 0, 1, 1);
	io_layout->addWidget(sx_set, 3, 0, 1, 1);
	io_layout->addWidget(ly_set, 4, 0, 1, 1);
	io_layout->addWidget(lyc_set, 5, 0, 1, 1);
	io_layout->addWidget(dma_set, 6, 0, 1, 1);
	io_layout->addWidget(bgp_set, 7, 0, 1, 1);
	io_layout->addWidget(obp0_set, 8, 0, 1, 1);
	io_layout->addWidget(obp1_set, 9, 0, 1, 1);
	io_layout->addWidget(wy_set, 10, 0, 1, 1);
	io_layout->addWidget(wx_set, 11, 0, 1, 1);
	io_layout->addWidget(nr10_set, 12, 0, 1, 1);
	io_layout->addWidget(nr11_set, 13, 0, 1, 1);

	io_layout->addWidget(nr12_set, 0, 1, 1, 1);
	io_layout->addWidget(nr13_set, 1, 1, 1, 1);
	io_layout->addWidget(nr14_set, 2, 1, 1, 1);
	io_layout->addWidget(nr21_set, 3, 1, 1, 1);
	io_layout->addWidget(nr22_set, 4, 1, 1, 1);
	io_layout->addWidget(nr23_set, 5, 1, 1, 1);
	io_layout->addWidget(nr24_set, 6, 1, 1, 1);
	io_layout->addWidget(nr30_set, 7, 1, 1, 1);
	io_layout->addWidget(nr31_set, 8, 1, 1, 1);
	io_layout->addWidget(nr32_set, 9, 1, 1, 1);
	io_layout->addWidget(nr33_set, 10, 1, 1, 1);
	io_layout->addWidget(nr34_set, 11, 1, 1, 1);
	io_layout->addWidget(nr41_set, 12, 1, 1, 1);
	io_layout->addWidget(nr42_set, 13, 1, 1, 1);

	io_layout->addWidget(nr43_set, 0, 2, 1, 1);
	io_layout->addWidget(nr44_set, 1, 2, 1, 1);
	io_layout->addWidget(nr50_set, 2, 2, 1, 1);
	io_layout->addWidget(nr51_set, 3, 2, 1, 1);
	io_layout->addWidget(nr52_set, 4, 2, 1, 1);
	io_layout->addWidget(key1_set, 5, 2, 1, 1);
	io_layout->addWidget(vbk_set, 6, 2, 1, 1);
	io_layout->addWidget(hdma1_set, 7, 2, 1, 1);
	io_layout->addWidget(hdma2_set, 8, 2, 1, 1);
	io_layout->addWidget(hdma3_set, 9, 2, 1, 1);
	io_layout->addWidget(hdma4_set, 10, 2, 1, 1);
	io_layout->addWidget(hdma5_set, 11, 2, 1, 1);
	io_layout->addWidget(rp_set, 12, 2, 1, 1);
	io_layout->addWidget(bcps_set, 13, 2, 1, 1);

	io_layout->addWidget(bcpd_set, 0, 3, 1, 1);
	io_layout->addWidget(ocps_set, 1, 3, 1, 1);
	io_layout->addWidget(ocpd_set, 2, 3, 1, 1);
	io_layout->addWidget(svbk_set, 3, 3, 1, 1);
	io_layout->addWidget(p1_set, 4, 3, 1, 1);
	io_layout->addWidget(sb_set, 5, 3, 1, 1);
	io_layout->addWidget(sc_set, 6, 3, 1, 1);
	io_layout->addWidget(div_set, 7, 3, 1, 1);
	io_layout->addWidget(tima_set, 8, 3, 1, 1);
	io_layout->addWidget(tma_set, 9, 3, 1, 1);
	io_layout->addWidget(tac_set, 10, 3, 1, 1);
	io_layout->addWidget(ie_set, 11, 3, 1, 1);
	io_layout->addWidget(if_set, 12, 3, 1, 1);

	io_regs->setLayout(io_layout);

	//Background palettes
	QWidget* bg_pal_set = new QWidget(palettes);
	bg_pal_set->setMaximumHeight(180);
	QLabel* bg_pal_label = new QLabel("Background Palettes", bg_pal_set);

	bg_pal_table = new QTableWidget(bg_pal_set);
	bg_pal_table->setRowCount(8);
	bg_pal_table->setColumnCount(4);
	bg_pal_table->setShowGrid(true);
	bg_pal_table->verticalHeader()->setVisible(false);
	bg_pal_table->verticalHeader()->setDefaultSectionSize(16);
	bg_pal_table->horizontalHeader()->setVisible(false);
	bg_pal_table->horizontalHeader()->setDefaultSectionSize(32);
	bg_pal_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	bg_pal_table->setSelectionMode(QAbstractItemView::NoSelection);
	bg_pal_table->setMaximumWidth(132);
	bg_pal_table->setMaximumHeight(132);

	QBrush blank_brush(Qt::black, Qt::BDiagPattern);

	for(u8 y = 0; y < 8; y++)
	{
		for(u8 x = 0; x < 4; x++)
		{
			bg_pal_table->setItem(y, x, new QTableWidgetItem);
			bg_pal_table->item(y, x)->setBackground(blank_brush);
		}
	}
	
	//Background palettes layout
	QVBoxLayout* bg_pal_layout = new QVBoxLayout;
	bg_pal_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	bg_pal_layout->addWidget(bg_pal_label);
	bg_pal_layout->addWidget(bg_pal_table);
	bg_pal_set->setLayout(bg_pal_layout);

	//OBJ palettes
	QWidget* obj_pal_set = new QWidget(palettes);
	obj_pal_set->setMaximumHeight(180);
	QLabel* obj_pal_label = new QLabel("OBJ Palettes", obj_pal_set);

	obj_pal_table = new QTableWidget(obj_pal_set);
	obj_pal_table->setRowCount(8);
	obj_pal_table->setColumnCount(4);
	obj_pal_table->setShowGrid(true);
	obj_pal_table->verticalHeader()->setVisible(false);
	obj_pal_table->verticalHeader()->setDefaultSectionSize(16);
	obj_pal_table->horizontalHeader()->setVisible(false);
	obj_pal_table->horizontalHeader()->setDefaultSectionSize(32);
	obj_pal_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	obj_pal_table->setSelectionMode(QAbstractItemView::NoSelection);
	obj_pal_table->setMaximumWidth(132);
	obj_pal_table->setMaximumHeight(132);

	for(u8 y = 0; y < 8; y++)
	{
		for(u8 x = 0; x < 4; x++)
		{
			obj_pal_table->setItem(y, x, new QTableWidgetItem);
			obj_pal_table->item(y, x)->setBackground(blank_brush);
		}
	}
	
	//OBJ palettes layout
	QVBoxLayout* obj_pal_layout = new QVBoxLayout;
	obj_pal_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	obj_pal_layout->addWidget(obj_pal_label);
	obj_pal_layout->addWidget(obj_pal_table);
	obj_pal_set->setLayout(obj_pal_layout);

	//BG palette preview
	QImage temp_pix(128, 128, QImage::Format_ARGB32);
	temp_pix.fill(qRgb(0, 0, 0));
	bg_pal_preview = new QLabel;
	bg_pal_preview->setPixmap(QPixmap::fromImage(temp_pix));

	//BG palettes labels + layout
	QWidget* bg_label_set = new QWidget(palettes);
	bg_r_label = new QLabel("R : \t", bg_label_set);
	bg_g_label = new QLabel("G : \t", bg_label_set);
	bg_b_label = new QLabel("B : \t", bg_label_set);
	bg_24_color = new QLabel("0x000000\t", bg_label_set);
	
	QVBoxLayout* bg_label_layout = new QVBoxLayout;
	bg_label_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	bg_label_layout->addWidget(bg_r_label);
	bg_label_layout->addWidget(bg_g_label);
	bg_label_layout->addWidget(bg_b_label);
	bg_label_layout->addWidget(bg_24_color);
	bg_label_set->setLayout(bg_label_layout);
	bg_label_set->setMaximumHeight(128);

	//OBJ palette preview
	obj_pal_preview = new QLabel;
	obj_pal_preview->setPixmap(QPixmap::fromImage(temp_pix));

	//OBJ palettes labels + layout
	QWidget* obj_label_set = new QWidget(palettes);
	obj_r_label = new QLabel("R : \t", obj_label_set);
	obj_g_label = new QLabel("G : \t", obj_label_set);
	obj_b_label = new QLabel("B : \t", obj_label_set);
	obj_24_color = new QLabel("0x000000\t", obj_label_set);
	
	QVBoxLayout* obj_label_layout = new QVBoxLayout;
	obj_label_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	obj_label_layout->addWidget(obj_r_label);
	obj_label_layout->addWidget(obj_g_label);
	obj_label_layout->addWidget(obj_b_label);
	obj_label_layout->addWidget(obj_24_color);
	obj_label_set->setLayout(obj_label_layout);
	obj_label_set->setMaximumHeight(128);
	
	//Palettes layout
	QGridLayout* palettes_layout = new QGridLayout;
	palettes_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	palettes_layout->addWidget(bg_pal_set, 0, 0, 1, 1);
	palettes_layout->addWidget(obj_pal_set, 1, 0, 1, 1);

	palettes_layout->addWidget(bg_pal_preview, 0, 1, 1, 1);
	palettes_layout->addWidget(obj_pal_preview, 1, 1, 1, 1);

	palettes_layout->addWidget(bg_label_set, 0, 2, 1, 2);
	palettes_layout->addWidget(obj_label_set, 1, 2, 1, 2);

	palettes->setLayout(palettes_layout);
	palettes->setMaximumWidth(600);

	//Memory
	QFont mono_font("Monospace");
	mono_font.setStyleHint(QFont::TypeWriter);

	QWidget* mem_set = new QWidget(memory);
	mem_set->setMinimumHeight(350);

	mem_addr = new QTextEdit(mem_set);
	mem_addr->setReadOnly(true);
	mem_addr->setFixedWidth(80);
	mem_addr->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mem_addr->setFont(mono_font);

	mem_values = new QTextEdit(mem_set);
	mem_values->setReadOnly(true);
	mem_values->setFixedWidth(500);
	mem_values->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mem_values->setTabStopWidth(28);
	mem_values->setFont(mono_font);

	mem_ascii = new QTextEdit(mem_set);
	mem_ascii->setReadOnly(true);
	mem_ascii->setFixedWidth(160);
	mem_ascii->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mem_ascii->setFont(mono_font);

	//ASCII lookup setup
	for(u8 x = 0; x < 0x20; x++) { ascii_lookup += "."; }
	
	ascii_lookup += " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

	for(u16 x = 0x7F; x < 0x100; x++) { ascii_lookup += "."; }

	//Memory QString setup
	for(u32 x = 0; x < 0xFFFF; x += 16)
	{
		if((x + 16) < 0xFFFF) { addr_text += (QString("%1").arg(x, 4, 16, QChar('0')).toUpper().prepend("0x").append("\n")); }
		else { addr_text += (QString("%1").arg(x, 4, 16, QChar('0')).toUpper().prepend("0x")); }
	}

	for(u32 x = 0; x < 0x10000; x++)
	{
		if((x % 16 == 0) && (x > 0))
		{
			values_text += (QString("%1").arg(0, 2, 16, QChar('0')).toUpper().append("\t").prepend("\n"));

			std::string ascii_string(".");
			ascii_text += QString::fromStdString(ascii_string).prepend("\n");
		}

		else
		{
			values_text += (QString("%1").arg(0, 2, 16, QChar('0')).toUpper().append("\t"));

			std::string ascii_string(".");
			ascii_text += QString::fromStdString(ascii_string);
		}
	}

	mem_addr->setText(addr_text);
	mem_values->setText(values_text);
	mem_ascii->setText(ascii_text);

	//Memory main scrollbar
	mem_scrollbar = new QScrollBar(mem_set);
	mem_scrollbar->setRange(0, 4095);
	mem_scrollbar->setOrientation(Qt::Vertical);

	//Memory layout
	QHBoxLayout* mem_layout = new QHBoxLayout;
	mem_layout->setSpacing(0);
	mem_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	mem_layout->addWidget(mem_addr);
	mem_layout->addWidget(mem_values);
	mem_layout->addWidget(mem_ascii);
	mem_layout->addWidget(mem_scrollbar);
	memory->setLayout(mem_layout);

	//Disassembler
	QWidget* dasm_set = new QWidget(cpu_instr);
	dasm_set->setMinimumHeight(350);

	counter = new QTextEdit(dasm_set);
	counter->setReadOnly(true);
	counter->setFixedWidth(80);
	counter->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	counter->setFont(mono_font);

	dasm = new QTextEdit(dasm_set);
	dasm->setReadOnly(true);
	dasm->setFixedWidth(500);
	dasm->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	dasm->setFont(mono_font);

	//Disassembler QString setup
	for(u32 x = 0; x < 0x10000; x++)
	{
		counter_text += QString("%1").arg(x, 4, 16, QChar('0')).toUpper().prepend("0x").append("\n");
	}

	counter->setText(counter_text);

	//Disassembler main scrollbar
	dasm_scrollbar = new QScrollBar(dasm_set);
	dasm_scrollbar->setRange(0, 0xFFFF);
	dasm_scrollbar->setOrientation(Qt::Vertical);

	//Disassembler layout
	QHBoxLayout* dasm_layout = new QHBoxLayout;
	dasm_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	dasm_layout->setSpacing(0);
	dasm_layout->addWidget(counter);
	dasm_layout->addWidget(dasm);
	dasm_layout->addWidget(dasm_scrollbar);
	dasm_set->setLayout(dasm_layout);

	//Register labels
	QWidget* regs_set = new QWidget(cpu_instr);
	regs_set->setFont(mono_font);
	af_label = new QLabel("A: 0x00    F: 0x00", regs_set);
	bc_label = new QLabel("B: 0x00    C: 0x00", regs_set);
	de_label = new QLabel("D: 0x00    E: 0x00", regs_set);
	hl_label = new QLabel("H: 0x00    L: 0x00", regs_set);
	sp_label = new QLabel("SP: 0x0000", regs_set);
	pc_label = new QLabel("PC: 0x0000", regs_set);
	flags_label = new QLabel("Flags: ZNHC", regs_set);

	db_next_button = new QPushButton("Next");
	db_set_bp_button = new QPushButton("Set Breakpoint");
	db_clear_bp_button = new QPushButton("Clear Breakpoints");
	db_continue_button = new QPushButton("Continue");
	db_reset_button = new QPushButton("Reset");
	db_reset_run_button = new QPushButton("Reset + Run");

	//Register layout
	QVBoxLayout* regs_layout = new QVBoxLayout;
	regs_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	regs_layout->setSpacing(4);
	regs_layout->addWidget(af_label);
	regs_layout->addWidget(bc_label);
	regs_layout->addWidget(de_label);
	regs_layout->addWidget(hl_label);
	regs_layout->addWidget(sp_label);
	regs_layout->addWidget(pc_label);
	regs_layout->addWidget(flags_label);
	regs_layout->addWidget(db_next_button);
	regs_layout->addWidget(db_continue_button);
	regs_layout->addWidget(db_set_bp_button);
	regs_layout->addWidget(db_clear_bp_button);
	regs_layout->addWidget(db_reset_button);
	regs_layout->addWidget(db_reset_run_button);
	regs_set->setLayout(regs_layout);
	
	QHBoxLayout* instr_layout = new QHBoxLayout;
	instr_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	instr_layout->setSpacing(0);
	instr_layout->addWidget(dasm_set);
	instr_layout->addWidget(regs_set);
	instr_layout->setStretchFactor(regs_set, 15);
	cpu_instr->setLayout(instr_layout);

	//Graphics - OBJ
	db_obj.clear();
	db_obj_label.clear();

	obj_large_preview = QImage(128, 128, QImage::Format_ARGB32);
	obj_large_label = new QLabel;
	obj_large_label->setPixmap(QPixmap::fromImage(obj_large_preview));

	for(int x = 0; x < 40; x++)
	{
		db_obj.push_back(QImage(64, 64, QImage::Format_ARGB32));

		QPushButton* temp_label = new QPushButton;
		temp_label->setIcon(QPixmap::fromImage(db_obj[x]));
		temp_label->setIconSize(QSize(32, 32));
		temp_label->setFlat(true);

		db_obj_label.push_back(temp_label);
	}

	QWidget* obj_info = new QWidget;
	obj_xy = new QLabel("X:    Y: ");
	obj_tile = new QLabel("Tile ID: ");
	obj_addr = new QLabel("Tile Address: ");
	obj_size = new QLabel("Tile Size: ");
	obj_flip = new QLabel("H-Flip:    V-Flip: ");
	obj_bank = new QLabel("VRAM Bank: ");
	obj_pal = new QLabel("Tile Palette: ");

	QVBoxLayout* obj_info_layout = new QVBoxLayout;
	obj_info_layout->addWidget(obj_xy);
	obj_info_layout->addWidget(obj_tile);
	obj_info_layout->addWidget(obj_addr);
	obj_info_layout->addWidget(obj_size);
	obj_info_layout->addWidget(obj_flip);
	obj_info_layout->addWidget(obj_pal);
	obj_info_layout->addWidget(obj_bank);
	obj_info->setLayout(obj_info_layout);

	//Graphics - OBJ layout
	QGridLayout* obj_layout = new QGridLayout;
	obj_layout->setVerticalSpacing(1);
	obj_layout->setHorizontalSpacing(2);

	for(int y = 0; y < 5; y++)
	{
		for(int x = 0; x < 8; x++)
		{
			obj_layout->addWidget(db_obj_label[(y*8) + x], y, x, 1, 1);
		}
	}

	obj_layout->addWidget(obj_large_label, 0, 9, -1, 2, Qt::AlignTop);
	obj_layout->addWidget(obj_info, 2, 9, -1, 2, Qt::AlignTop);
	vram_obj->setLayout(obj_layout);

	//Graphics - BG
	db_bg.clear();
	db_bg_label.clear();

	bg_large_preview = QImage(128, 128, QImage::Format_ARGB32);
	bg_large_label = new QLabel;
	bg_large_label->setPixmap(QPixmap::fromImage(bg_large_preview));

	for(int x = 0; x < 256; x++)
	{
		db_bg.push_back(QImage(64, 64, QImage::Format_ARGB32));

		QPushButton* temp_label = new QPushButton;
		temp_label->setIcon(QPixmap::fromImage(db_bg[x]));
		temp_label->setIconSize(QSize(16, 16));
		temp_label->setFlat(true);

		db_bg_label.push_back(temp_label);
	}

	//Graphics - BG layout
	QGridLayout* bg_layout = new QGridLayout;
	bg_layout->setVerticalSpacing(0);
	bg_layout->setHorizontalSpacing(0);

	for(int y = 0; y < 16; y++)
	{
		for(int x = 0; x < 16; x++)
		{
			bg_layout->addWidget(db_bg_label[(y*16) + x], y, x, 1, 1);
		}
	}

	bg_layout->addWidget(bg_large_label, 0, 17, -1, 2, Qt::AlignTop);
	vram_bg->setLayout(bg_layout);

	refresh_button = new QPushButton("Refresh");
	tabs_button = new QDialogButtonBox(QDialogButtonBox::Close);
	tabs_button->addButton(refresh_button, QDialogButtonBox::ActionRole);

	//Final tab layout
	QVBoxLayout* main_layout = new QVBoxLayout;
	main_layout->addWidget(tabs);
	main_layout->addWidget(tabs_button);
	setLayout(main_layout);

	connect(tabs_button, SIGNAL(accepted()), this, SLOT(accept()));
	connect(tabs_button, SIGNAL(rejected()), this, SLOT(reject()));
	connect(bg_pal_table, SIGNAL(cellClicked (int, int)), this, SLOT(preview_bg_color(int, int)));
	connect(obj_pal_table, SIGNAL(cellClicked (int, int)), this, SLOT(preview_obj_color(int, int)));
	connect(mem_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scroll_mem(int)));
	connect(dasm_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scroll_dasm(int)));
	connect(dasm, SIGNAL(cursorPositionChanged()), this, SLOT(highlight()));
	connect(db_next_button, SIGNAL(clicked()), this, SLOT(db_next()));
	connect(db_continue_button, SIGNAL(clicked()), this, SLOT(db_continue()));
	connect(db_clear_bp_button, SIGNAL(clicked()), this, SLOT(db_clear_bp()));
	connect(db_set_bp_button, SIGNAL(clicked()), this, SLOT(db_set_bp()));
	connect(db_reset_button, SIGNAL(clicked()), this, SLOT(db_reset()));
	connect(db_reset_run_button, SIGNAL(clicked()), this, SLOT(db_reset_run()));
	connect(refresh_button, SIGNAL(clicked()), this, SLOT(click_refresh()));
	connect(tabs_button->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close_debug()));

	//Signal mapper for memory scrollbars
	QSignalMapper* text_mapper = new QSignalMapper(this);
	connect(mem_addr->verticalScrollBar(), SIGNAL(valueChanged(int)), text_mapper, SLOT(map()));
	connect(mem_values->verticalScrollBar(), SIGNAL(valueChanged(int)), text_mapper, SLOT(map()));
	connect(mem_ascii->verticalScrollBar(), SIGNAL(valueChanged(int)), text_mapper, SLOT(map()));

	text_mapper->setMapping(mem_addr->verticalScrollBar(), 0);
	text_mapper->setMapping(mem_values->verticalScrollBar(), 1);
	text_mapper->setMapping(mem_ascii->verticalScrollBar(), 2);
	connect(text_mapper, SIGNAL(mapped(int)), this, SLOT(scroll_text(int)));

	//Signal mapper for disassembly scrollbars
	QSignalMapper* dasm_mapper = new QSignalMapper(this);
	connect(counter->verticalScrollBar(), SIGNAL(valueChanged(int)), dasm_mapper, SLOT(map()));
	connect(dasm->verticalScrollBar(), SIGNAL(valueChanged(int)), dasm_mapper, SLOT(map()));

	dasm_mapper->setMapping(counter->verticalScrollBar(), 0);
	dasm_mapper->setMapping(dasm->verticalScrollBar(), 1);
	connect(dasm_mapper, SIGNAL(mapped(int)), this, SLOT(scroll_count(int)));

	//Signal mapper for OBJ tiles
	QSignalMapper* obj_mapper = new QSignalMapper(this);
	
	for(int x = 0; x < 40; x++)
	{
		connect(db_obj_label[x], SIGNAL(clicked()), obj_mapper, SLOT(map()));
		obj_mapper->setMapping(db_obj_label[x], x);
	}

	connect(obj_mapper, SIGNAL(mapped(int)), this, SLOT(show_obj(int)));

	//Signal mapper for BG tiles
	QSignalMapper* bg_mapper = new QSignalMapper(this);
	
	for(int x = 0; x < 256; x++)
	{
		connect(db_bg_label[x], SIGNAL(clicked()), bg_mapper, SLOT(map()));
		bg_mapper->setMapping(db_bg_label[x], x);
	}

	connect(bg_mapper, SIGNAL(mapped(int)), this, SLOT(show_bg(int)));

	resize(800, 500);
	setWindowTitle(tr("DMG-GBC Debugger"));

	debug_reset = true;
	text_select = 0;
	pause = false;
	old_pause = false;
}

/****** Closes the debugging window ******/
void dmg_debug::closeEvent(QCloseEvent* event) { close_debug(); }

/****** Closes the debugging window ******/
void dmg_debug::close_debug() 
{
	hide();

	qt_gui::draw_surface->findChild<QAction*>("pause_action")->setEnabled(true);

	if(main_menu::gbe_plus->db_unit.last_command != "c")
	{
		main_menu::gbe_plus->db_unit.debug_mode = false;
		main_menu::gbe_plus->db_unit.last_command = "dq";
	}

	else
	{
		main_menu::gbe_plus->db_unit.debug_mode = false;
		main_menu::gbe_plus->db_unit.last_command = "dq";

		main_menu::dmg_debugger->pause = false;
		config::pause_emu = main_menu::dmg_debugger->old_pause;
		qt_gui::draw_surface->pause_emu();
	}

	//Clear existing breakpoints on close
	main_menu::dmg_debugger->dasm->setText(main_menu::dmg_debugger->dasm_text);
	main_menu::gbe_plus->db_unit.breakpoints.clear();

	//Clear format manually
	QTextCursor cursor(dasm->textCursor());
	cursor.setPosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.setBlockFormat(original_format);
}

/****** Refresh the display data ******/
void dmg_debug::refresh()
{
	u16 temp = 0;
	std::string temp_str = "";

	//Update I/O regs
	temp = main_menu::gbe_plus->ex_read_u8(REG_LCDC);
	mmio_lcdc->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_STAT);
	mmio_stat->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_SY);
	mmio_sy->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_SX);
	mmio_sx->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_LY);
	mmio_ly->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_LYC);
	mmio_lyc->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_DMA);
	mmio_dma->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_BGP);
	mmio_bgp->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_OBP0);
	mmio_obp0->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_OBP1);
	mmio_obp1->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_WY);
	mmio_wy->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_WX);
	mmio_wx->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR10);
	mmio_nr10->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR11);
	mmio_nr11->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR12);
	mmio_nr12->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR13);
	mmio_nr13->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR14);
	mmio_nr14->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR21);
	mmio_nr21->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR22);
	mmio_nr22->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR23);
	mmio_nr23->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR24);
	mmio_nr24->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR30);
	mmio_nr30->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR31);
	mmio_nr31->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR32);
	mmio_nr32->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR33);
	mmio_nr33->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR34);
	mmio_nr34->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR41);
	mmio_nr41->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR42);
	mmio_nr42->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR43);
	mmio_nr43->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR44);
	mmio_nr44->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR50);
	mmio_nr50->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR51);
	mmio_nr51->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(NR52);
	mmio_nr52->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_VBK);
	mmio_vbk->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_KEY1);
	mmio_key1->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_HDMA1);
	mmio_hdma1->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_HDMA2);
	mmio_hdma2->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_HDMA3);
	mmio_hdma3->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_HDMA4);
	mmio_hdma4->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_HDMA5);
	mmio_hdma5->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_RP);
	mmio_rp->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_BCPS);
	mmio_bcps->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_BCPD);
	mmio_bcpd->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_OCPS);
	mmio_ocps->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_OCPD);
	mmio_ocpd->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_SVBK);
	mmio_svbk->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_DIV);
	mmio_div->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_TIMA);
	mmio_tima->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_TMA);
	mmio_tma->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_TAC);
	mmio_tac->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_P1);
	mmio_p1->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_SB);
	mmio_sb->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(REG_SC);
	mmio_sc->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(IE_FLAG);
	mmio_ie->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	temp = main_menu::gbe_plus->ex_read_u8(IF_FLAG);
	mmio_if->setText(QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().prepend("0x"));

	//DMG Palettes
	if(config::gb_type != 2)
	{
		//BG
		u8 ex_bgp[4];
		temp = main_menu::gbe_plus->ex_read_u8(REG_BGP);

		ex_bgp[0] = temp  & 0x3;
		ex_bgp[1] = (temp >> 2) & 0x3;
		ex_bgp[2] = (temp >> 4) & 0x3;
		ex_bgp[3] = (temp >> 6) & 0x3;
	
		for(u8 x = 0; x < 4; x++)
		{
			switch(ex_bgp[x])
			{
				case 0: bg_pal_table->item(0, x)->setBackground(QColor(0xFF, 0xFF, 0xFF)); break;
				case 1: bg_pal_table->item(0, x)->setBackground(QColor(0xC0, 0xC0, 0xC0)); break;
				case 2: bg_pal_table->item(0, x)->setBackground(QColor(0x60, 0x60, 0x60)); break;
				case 3: bg_pal_table->item(0, x)->setBackground(QColor(0, 0, 0)); break;
			}
		}

		//OBJ
		u8 ex_obp[4][2];
		temp = main_menu::gbe_plus->ex_read_u8(REG_OBP0);

		ex_obp[0][0] = temp  & 0x3;
		ex_obp[1][0] = (temp >> 2) & 0x3;
		ex_obp[2][0] = (temp >> 4) & 0x3;
		ex_obp[3][0] = (temp >> 6) & 0x3;

		temp = main_menu::gbe_plus->ex_read_u8(REG_OBP1);

		ex_obp[0][1] = temp  & 0x3;
		ex_obp[1][1] = (temp >> 2) & 0x3;
		ex_obp[2][1] = (temp >> 4) & 0x3;
		ex_obp[3][1] = (temp >> 6) & 0x3;

		for(u8 y = 0; y < 2; y++)
		{
			for(u8 x = 0; x < 4; x++)
			{
				switch(ex_obp[x][y])
				{
					case 0: obj_pal_table->item(y, x)->setBackground(QColor(0xFF, 0xFF, 0xFF)); break;
					case 1: obj_pal_table->item(y, x)->setBackground(QColor(0xC0, 0xC0, 0xC0)); break;
					case 2: obj_pal_table->item(y, x)->setBackground(QColor(0x60, 0x60, 0x60)); break;
					case 3: obj_pal_table->item(y, x)->setBackground(QColor(0, 0, 0)); break;
				}
			}
		}
	}

	//GBC Palettes
	else
	{
		/*
		//BG
		for(u8 y = 0; y < 8; y++)
		{
			u32* color = main_menu::gbe_plus->get_bg_palette(y);

			for(u8 x = 0; x < 4; x++)
			{
				u8 blue = (*color) & 0xFF;
				u8 green = ((*color) >> 8) & 0xFF;
				u8 red = ((*color) >> 16) & 0xFF;

				bg_pal_table->item(y, x)->setBackground(QColor(red, green, blue));
				color += 8;
			}
		}

		//OBJ
		for(u8 y = 0; y < 8; y++)
		{
			u32* color = main_menu::gbe_plus->get_obj_palette(y);

			for(u8 x = 0; x < 4; x++)
			{
				u8 blue = (*color) & 0xFF;
				u8 green = ((*color) >> 8) & 0xFF;
				u8 red = ((*color) >> 16) & 0xFF;

				obj_pal_table->item(y, x)->setBackground(QColor(red, green, blue));
				color += 8;
			}
		}
		*/
	}

	//Memory viewer
	values_text.clear();
	ascii_text.clear();

	for(u32 x = 0; x < 0x10000; x++)
	{
		temp = main_menu::gbe_plus->ex_read_u8(x); 

		if((x % 16 == 0) && (x > 0))
		{
			values_text += (QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().append("\t").prepend("\n"));

			std::string ascii_string = ascii_lookup.substr(temp, 1);
			ascii_text += QString::fromStdString(ascii_string).prepend("\n");
		}

		else
		{
			values_text += (QString("%1").arg(temp, 2, 16, QChar('0')).toUpper().append("\t"));

			std::string ascii_string = ascii_lookup.substr(temp, 1);
			ascii_text += QString::fromStdString(ascii_string);
		}
	}

	mem_values->setText(values_text);
	mem_ascii->setText(ascii_text);

	if(debug_reset)
	{
		dasm_text.clear();

		//Populate initial disassembly text
		for(u32 x = 0; x < 0x10000; x++)
		{
			temp = main_menu::gbe_plus->ex_read_u8(x);
			temp_str = util::to_hex_str(temp);

			temp_str += " --> " + main_menu::gbe_plus->debug_get_mnemonic(x) + "\n";
			dasm_text += QString::fromStdString(temp_str);
		}

		dasm->setText(dasm_text);
		debug_reset = false;
		refresh();

		original_format = dasm->textCursor().blockFormat();
	}

	//Update register values
	temp_str = "A: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(0)) + "    F: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(7));
	af_label->setText(QString::fromStdString(temp_str));

	temp_str = "B: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(1)) + "    C: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(2));
	bc_label->setText(QString::fromStdString(temp_str));

	temp_str = "D: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(3)) + "    E: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(4));
	de_label->setText(QString::fromStdString(temp_str));

	temp_str = "H: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(5)) + "    L: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(6));
	hl_label->setText(QString::fromStdString(temp_str));

	temp_str = "SP: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(8));
	sp_label->setText(QString::fromStdString(temp_str));

	temp_str = "PC: " + util::to_hex_str(main_menu::gbe_plus->ex_get_reg(9));
	pc_label->setText(QString::fromStdString(temp_str));

	temp_str = "Flags: ";
	temp = main_menu::gbe_plus->ex_get_reg(7);

	temp_str += (temp & 0x80) ? "Z" : ".";
	temp_str += (temp & 0x40) ? "N" : ".";
	temp_str += (temp & 0x20) ? "H" : ".";
	temp_str += (temp & 0x10) ? "C" : ".";

	flags_label->setText(QString::fromStdString(temp_str));

	//Scroll disassembly to the PC
	temp = main_menu::gbe_plus->ex_get_reg(9);

	scroll_dasm(temp);
	text_select = true;
	highlight();

	if(main_menu::gbe_plus->db_unit.last_command != "c") { main_menu::gbe_plus->db_unit.last_command = ""; }

	//Update OBJ
	for(int x = 0; x < 40; x++)
	{
		//db_obj[x] = qt_gui::draw_surface->cgfx->grab_obj_data(x);
		//db_obj_label[x]->setIcon(QPixmap::fromImage(db_obj[x]));
	}

	//Update BG
	for(int x = 0; x < 256; x++)
	{
		//db_bg[x] = qt_gui::draw_surface->cgfx->grab_bg_data(x);
		//db_bg_label[x]->setIcon(QPixmap::fromImage(db_bg[x]));
	}
}

/****** Updates the debugger when clicking the Refresh button ******/
void dmg_debug::click_refresh()
{
	if(tabs->currentIndex() == 3) { debug_reset = true; }
	refresh();

	//Restore highlighting in the disassembly if necessary
	if(tabs->currentIndex() == 3)
	{
		for(int x = 0; x < main_menu::gbe_plus->db_unit.breakpoints.size(); x++)
		{
			u32 breakpoint = main_menu::gbe_plus->db_unit.breakpoints[x];
			QTextCursor cursor(dasm->document()->findBlockByLineNumber(breakpoint));
			QTextBlockFormat format = cursor.blockFormat();

			format.setBackground(QColor(Qt::yellow));
			cursor.setBlockFormat(format);
		}
	}	
}

/****** Updates a preview of the selected BG Color ******/
void dmg_debug::preview_bg_color(int y, int x)
{
	QImage temp_pix(128, 128, QImage::Format_ARGB32);
	temp_pix.fill(bg_pal_table->item(y, x)->background().color());
	bg_pal_preview->setPixmap(QPixmap::fromImage(temp_pix));

	int r, g, b = 0;
	bg_pal_table->item(y, x)->background().color().getRgb(&r, &g, &b);

	u32 color_word = (r << 16) | (g << 8) | b;
	std::string color_hex = util::to_hex_str(color_word, 3) + "\t";

	bg_r_label->setText(QString::number(r/8).prepend("R : ").append("\t"));
	bg_g_label->setText(QString::number(g/8).prepend("G : ").append("\t"));
	bg_b_label->setText(QString::number(b/8).prepend("B : ").append("\t"));
	bg_24_color->setText(QString::fromStdString(color_hex));
}

/****** Updates a preview of the selected OBJ Color ******/
void dmg_debug::preview_obj_color(int y, int x)
{
	QImage temp_pix(128, 128, QImage::Format_ARGB32);
	temp_pix.fill(obj_pal_table->item(y, x)->background().color());
	obj_pal_preview->setPixmap(QPixmap::fromImage(temp_pix));

	int r, g, b = 0;
	obj_pal_table->item(y, x)->background().color().getRgb(&r, &g, &b);

	u32 color_word = (r << 16) | (g << 8) | b;
	std::string color_hex = util::to_hex_str(color_word, 3) + "\t";

	obj_r_label->setText(QString::number(r/8).prepend("R : ").append("\t"));
	obj_g_label->setText(QString::number(g/8).prepend("G : ").append("\t"));
	obj_b_label->setText(QString::number(b/8).prepend("B : ").append("\t"));
	obj_24_color->setText(QString::fromStdString(color_hex));
}

/****** Scrolls everything in the memory tab via main scrollbar ******/
void dmg_debug::scroll_mem(int value)
{
	mem_addr->setTextCursor(QTextCursor(mem_addr->document()->findBlockByLineNumber(value)));
	mem_values->setTextCursor(QTextCursor(mem_values->document()->findBlockByLineNumber(value)));
	mem_ascii->setTextCursor(QTextCursor(mem_ascii->document()->findBlockByLineNumber(value)));
	mem_scrollbar->setValue(value);
}

/****** Scrolls every QTextEdit in the memory tab ******/
void dmg_debug::scroll_text(int type) 
{
	int line_number = 0;
	int value = 0;

	//Scroll based on Address
	if(type == 0)
	{
		line_number = mem_addr->textCursor().blockNumber();
		mem_scrollbar->setValue(line_number);

		value = mem_addr->verticalScrollBar()->value();
		mem_values->verticalScrollBar()->setValue(value);
		mem_ascii->verticalScrollBar()->setValue(value);
	}

	//Scroll based on Values
	else if(type == 1)
	{
		line_number = mem_values->textCursor().blockNumber();
		mem_scrollbar->setValue(line_number);

		value = mem_values->verticalScrollBar()->value();
		mem_addr->verticalScrollBar()->setValue(value);
		mem_ascii->verticalScrollBar()->setValue(value);
	}

	//Scroll based on ASCII
	else if(type == 2)
	{
		int line_number = mem_ascii->textCursor().blockNumber();
		mem_scrollbar->setValue(line_number);

		value = mem_ascii->verticalScrollBar()->value();
		mem_addr->verticalScrollBar()->setValue(value);
		mem_values->verticalScrollBar()->setValue(value);
	}
}

/****** Scrolls all everything in the disassembly tab via main scrollbar ******/
void dmg_debug::scroll_dasm(int value)
{
	text_select = false;
	counter->setTextCursor(QTextCursor(counter->document()->findBlockByLineNumber(value)));
	dasm->setTextCursor(QTextCursor(dasm->document()->findBlockByLineNumber(value)));
	dasm_scrollbar->setValue(value);

	highlighted_dasm_line = value;
}

/****** Scrolls every QTextEdit in the disassembly tab ******/
void dmg_debug::scroll_count(int type)
{
	int line_number = 0;
	int value = 0;

	//Scroll based on Counter
	if(type == 0)
	{
		line_number = counter->textCursor().blockNumber();
		dasm_scrollbar->setValue(line_number);

		value = counter->verticalScrollBar()->value();
		dasm->verticalScrollBar()->setValue(value);
	}

	//Scroll based on Disassembly
	else if(type == 1)
	{
		line_number = dasm->textCursor().blockNumber();
		dasm_scrollbar->setValue(line_number);

		value = dasm->verticalScrollBar()->value();
		counter->verticalScrollBar()->setValue(value);
	}
}

/****** Highlights a block of text for the disassembler ******/
void dmg_debug::highlight()
{
	if(text_select)
	{
		QTextCursor cursor = dasm->textCursor();
		int start_pos = cursor.block().position();
		int stop_pos = start_pos + cursor.block().length() - 1;

		cursor.setPosition(start_pos);
		cursor.setPosition(stop_pos, QTextCursor::KeepAnchor);

		dasm->setTextCursor(cursor);
	}

	else { text_select = true; }

	highlighted_dasm_line = dasm->textCursor().blockNumber();
}

/****** Updates the OBJ preview ******/
void dmg_debug::show_obj(int obj_id)
{
	u8 temp = main_menu::gbe_plus->ex_read_u8(REG_LCDC) & 0x4;
	std::string obj_text = "";

	//Update OBJ size
	if(temp == 0)
	{
		//obj_large_preview = qt_gui::draw_surface->cgfx->grab_obj_data(obj_id).scaled(128, 128);
		obj_text = "Tile Size: 8x8";
		obj_size->setText(QString::fromStdString(obj_text));
	}

	else
	{
		//obj_large_preview = qt_gui::draw_surface->cgfx->grab_obj_data(obj_id).scaled(64, 128);
		obj_text = "Tile Size: 8x16";
		obj_size->setText(QString::fromStdString(obj_text));
	}

	//obj_large_label->setPixmap(QPixmap::fromImage(obj_large_preview));

	//Update X/Y coordinate
	u16 vram_addr = 0xFE00 + (4 * obj_id);

	u8 oam_x = main_menu::gbe_plus->ex_read_u8(vram_addr + 1);
	u8 oam_y = main_menu::gbe_plus->ex_read_u8(vram_addr);

	oam_x -= 8;
	oam_y -= 16;

	obj_text = "X: " + util::to_str(oam_x) + "  Y: " + util::to_str(oam_y);
	obj_xy->setText(QString::fromStdString(obj_text));

	//Update Tile ID
	if(temp == 0) { temp = main_menu::gbe_plus->ex_read_u8(vram_addr + 2); }
	else { temp = main_menu::gbe_plus->ex_read_u8(vram_addr + 2) & ~0x1; }

	obj_text = "Tile #: " + util::to_str(temp);
	obj_tile->setText(QString::fromStdString(obj_text));

	//Update Tile Address
	obj_text = "Tile Address: " + util::to_hex_str(0x8000 + (16 * temp));
	obj_addr->setText(QString::fromStdString(obj_text));

	//Update Flipping
	temp = main_menu::gbe_plus->ex_read_u8(vram_addr + 3);

	bool h_flip = temp & 0x20;
	bool v_flip = temp & 0x40;

	if((!h_flip) && (!v_flip)) { obj_text = "H-Flip : N    V-Flip : N"; }
	else if((h_flip) && (!v_flip)) { obj_text = "H-Flip : Y    V-Flip : N"; }
	else if((!h_flip) && (v_flip)) { obj_text = "H-Flip : N    V-Flip : Y"; }
	else { obj_text = "H-Flip : Y    V-Flip : Y"; }

	obj_flip->setText(QString::fromStdString(obj_text));

	//Update VRAM bank
	if(config::gb_type < 2) { obj_text = "VRAM Bank: N/A"; }
	else { obj_text = "VRAM Bank: " + util::to_str((temp >> 3) & 0x1); }

	obj_bank->setText(QString::fromStdString(obj_text));

	//Update Palette
	if(config::gb_type < 2) 
	{
		temp = (temp >> 4) & 0x1;
		
		if(temp == 0) { obj_text = "Tile Palette: OBP0"; }
		else { obj_text = "Tile Palette: OBP1"; }
	}
	
	else
	{
		temp &= 0x7;
		obj_text = "Tile Palette : OCP" + util::to_str(temp);
	}

	obj_pal->setText(QString::fromStdString(obj_text));
}

/****** Updates the BG preview ******/
void dmg_debug::show_bg(int bg_id)
{
	//bg_large_preview = qt_gui::draw_surface->cgfx->grab_bg_data(bg_id).scaled(128, 128);
	//bg_large_label->setPixmap(QPixmap::fromImage(bg_large_preview));
}

/****** Automatically refresh display data - Call this publically ******/
void dmg_debug::auto_refresh() { refresh(); }

/****** Clears the formatting from all breakpoints - Call this publically ******/
void dmg_debug::clear_format()
{
	QTextCursor cursor(dasm->textCursor());
	cursor.setPosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.setBlockFormat(original_format);
	
	refresh();
}

/****** Moves the debugger one instruction in disassembly ******/
void dmg_debug::db_next() { main_menu::gbe_plus->db_unit.last_command = "n"; }

/****** Continues emulation until debugger hits breakpoint ******/
void dmg_debug::db_continue() { main_menu::gbe_plus->db_unit.last_command = "c"; }

/****** Sets breakpoint at current PC ******/
void dmg_debug::db_set_bp() 
{
	if(main_menu::gbe_plus->db_unit.last_command != "c") { main_menu::gbe_plus->db_unit.last_command = "bp"; }
}

/****** Clears all breakpoints ******/
void dmg_debug::db_clear_bp()
{
	if(main_menu::gbe_plus->db_unit.last_command != "c") { main_menu::gbe_plus->db_unit.last_command = "cbp"; }
}

/****** Resets emulation then stops ******/
void dmg_debug::db_reset() { main_menu::gbe_plus->db_unit.last_command = "rs"; }

/****** Resets emulation and runs in continue mode ******/
void dmg_debug::db_reset_run() { main_menu::gbe_plus->db_unit.last_command = "rsr"; }

/****** Steps through the debugger via the GUI ******/
void dmg_debug_step()
{
	bool halt = true;
	bool bp_continue = true;

	//Continue until breakpoint
	if(main_menu::gbe_plus->db_unit.last_command == "c")
	{
		if(main_menu::gbe_plus->db_unit.breakpoints.size() > 0)
		{
			for(int x = 0; x < main_menu::gbe_plus->db_unit.breakpoints.size(); x++)
			{
				//When a BP is matched, display info, wait for next input command
				if(main_menu::gbe_plus->ex_get_reg(9) == main_menu::gbe_plus->db_unit.breakpoints[x])
				{
					main_menu::gbe_plus->db_unit.last_command = "";
					bp_continue = false;
				}
			}
		}

		if(bp_continue) { return; }
	}

	main_menu::dmg_debugger->auto_refresh();

	//Wait for GUI action
	while(((main_menu::gbe_plus->db_unit.last_command == "") || (main_menu::gbe_plus->db_unit.last_command == "dq")) && (halt))
	{
		SDL_Delay(16);
		QApplication::processEvents();
		if(SDL_GetAudioStatus() != SDL_AUDIO_PAUSED) { SDL_PauseAudio(1); }

		//Step once
		if(main_menu::gbe_plus->db_unit.last_command == "n") 
		{
			halt = false;
		}

		//Set breakpoint at current PC
		else if(main_menu::gbe_plus->db_unit.last_command == "bp")
		{
			main_menu::gbe_plus->db_unit.breakpoints.push_back(main_menu::dmg_debugger->highlighted_dasm_line);
			main_menu::gbe_plus->db_unit.last_command = "";

			QTextCursor cursor(main_menu::dmg_debugger->dasm->textCursor());
			QTextBlockFormat format = cursor.blockFormat();

			format.setBackground(QColor(Qt::yellow));
			cursor.setBlockFormat(format);
		}

		//Clears all breakpoints
		else if(main_menu::gbe_plus->db_unit.last_command == "cbp")
		{
			main_menu::dmg_debugger->dasm->setText(main_menu::dmg_debugger->dasm_text);
			main_menu::gbe_plus->db_unit.last_command = "";
			main_menu::gbe_plus->db_unit.breakpoints.clear();
			main_menu::dmg_debugger->clear_format();
		}

		//Resets debugging, then stops
		else if(main_menu::gbe_plus->db_unit.last_command == "rs")
		{
			main_menu::gbe_plus->db_unit.last_command = "";
			main_menu::gbe_plus->reset();
			main_menu::dmg_debugger->auto_refresh();
		}

		//Resets debugging, then runs
		else if(main_menu::gbe_plus->db_unit.last_command == "rsr")
		{
			main_menu::gbe_plus->db_unit.last_command = "c";
			main_menu::gbe_plus->reset();
		}

		//Stop debugging
		else if(main_menu::gbe_plus->db_unit.last_command == "dq") 
		{
			halt = false;
			main_menu::dmg_debugger->pause = false;
			config::pause_emu = main_menu::dmg_debugger->old_pause;

			if(!config::pause_emu) { SDL_PauseAudio(0); }

			qt_gui::draw_surface->pause_emu();
		}

		//Continue waiting for a valid debugging command
		else { halt = true; }
	}
}
