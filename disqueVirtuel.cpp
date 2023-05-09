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
		int succes = 1;

		try
		{
			std::vector<bool> initalisateurBlock(N_BLOCK_ON_DISK, true);
			for (int i = 0; i < (N_INODE_ON_DISK + FREE_INODE_BITMAP); i++)
			{
				initalisateurBlock.at(i) = false;
			}

			std::vector<bool> initalisateurInode(N_INODE_ON_DISK, true);
			*initalisateurInode.begin() = false;

			this->m_blockDisque.at(FREE_BLOCK_BITMAP).setBitmap(initalisateurBlock);
			this->m_blockDisque.at(FREE_INODE_BITMAP).setBitmap(initalisateurInode);

			for (int i = BASE_BLOCK_INODE; i < (N_INODE_ON_DISK + BASE_BLOCK_INODE); i++)
			{
				this->m_blockDisque.at(i).m_inode = new iNode(i-BASE_BLOCK_INODE, 0, 0, 0, 0); // TODO delete inodes in desctuctor
			}

			for (int i = 0; i < this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.size(); i++)
			{
				if (this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(i))
				{
					// TODO Increment stuff?

					this->m_blockDisque.at(i).m_dirEntry.push_back(new dirEntry(1, "."));
					this->m_blockDisque.at(i).m_dirEntry.push_back(new dirEntry(1, ".."));

					this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(i) = false;
					this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(1) = false;

					this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_mode = S_IFDIR;
					this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_nlink = 2; //sus
					this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_block = i;
					
					break;
				}
			}
		}
		catch (const std::exception &e)
		{
			succes = 0;
		}

		return succes;
	}

	int DisqueVirtuel::bd_mkdir(const std::string &p_DirName)
	{
		return 0;
	}

	int DisqueVirtuel::bd_create(const std::string &p_FileName)
	{
		return 0;
	}

	std::string DisqueVirtuel::bd_ls(const std::string& p_DirLocation)
	{
		return "_";
	}

	int DisqueVirtuel::bd_rm(const std::string &p_Filename)
	{
		return 0;
	}

	DisqueVirtuel::~DisqueVirtuel() {}

} // Fin du namespace
