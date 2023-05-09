/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author ?
 * \version 0.1
 * \date  2022
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <iostream>
#include <string>
// vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{

	DisqueVirtuel::DisqueVirtuel()
	{
		TP3::Block nouveauBlockReserve;
		for (int i = 0; i < FREE_BLOCK_BITMAP; i++)
		{
			this->m_blockDisque.push_back(nouveauBlockReserve);
		}

		TP3::Block nouveauBlockBlockBitmap(S_IFBL);
		TP3::Block nouveauBlockInodeBitmap(S_IFIL);
		this->m_blockDisque.push_back(nouveauBlockBlockBitmap);
		this->m_blockDisque.push_back(nouveauBlockInodeBitmap);

		TP3::Block nouveauBlockDeInode(S_IFIN);
		for (int i = 0; i < N_INODE_ON_DISK; i++)
		{
			this->m_blockDisque.push_back(nouveauBlockDeInode);
		}

		TP3::Block nouveauBlockDeDonnee(S_IFDE);
		while (this->m_blockDisque.size() < N_BLOCK_ON_DISK)
		{
			this->m_blockDisque.push_back(nouveauBlockDeDonnee);
		}
	}

	int DisqueVirtuel::bd_FormatDisk()
	{
		for (auto block_iter : this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap)
		{
			block_iter = 1;
		}

		for (auto inode_iter : this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap)
		{
			//
		}

		return 1;
	}

} // Fin du namespace
