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
#include <vector>
#include <sstream>
// vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	DisqueVirtuel::DisqueVirtuel()
	{
		// Ajout des blocks 0 et 1
		TP3::Block nouveauBlockReserve;
		for (int i = 0; i < FREE_BLOCK_BITMAP; i++)
		{
			this->m_blockDisque.push_back(nouveauBlockReserve);
		}

		// Ajout des blocks de bitmap
		TP3::Block nouveauBlockBlockBitmap(S_IFBL);
		TP3::Block nouveauBlockInodeBitmap(S_IFIL);
		this->m_blockDisque.push_back(nouveauBlockBlockBitmap);
		this->m_blockDisque.push_back(nouveauBlockInodeBitmap);

		// Ajout des blocks contenant les i-nodes
		TP3::Block nouveauBlockDeInode(S_IFIN);
		for (int i = 0; i < N_INODE_ON_DISK; i++)
		{
			this->m_blockDisque.push_back(nouveauBlockDeInode);
		}

		// Ajout des blocks de données
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
			// On marque les blocks 0 à 23 comme non-libres
			for (int i = 0; i < (N_INODE_ON_DISK + FREE_INODE_BITMAP); i++)
			{
				initalisateurBlock.at(i) = false;
			}

			std::vector<bool> initalisateurInode(N_INODE_ON_DISK, true);
			// On marque le premier i-node comme non-libre
			*initalisateurInode.begin() = false;

			this->m_blockDisque.at(FREE_BLOCK_BITMAP).setBitmap(initalisateurBlock);
			this->m_blockDisque.at(FREE_INODE_BITMAP).setBitmap(initalisateurInode);

			for (int i = BASE_BLOCK_INODE; i < (N_INODE_ON_DISK + BASE_BLOCK_INODE); i++)
			{
				this->m_blockDisque.at(i).m_inode = new iNode(i - BASE_BLOCK_INODE, 0, 0, 0, 0); // TODO delete inodes in desctuctor
			}

			/* for (int i = 0; i < this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.size(); i++)
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
			} */

			int blockAUtiliser = bd_findFreeBlock();
			m_blockRoot = blockAUtiliser;

			this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(blockAUtiliser) = false;
			this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(1) = false;

			this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_mode = S_IFDIR;
			this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_nlink = 2; // sus
			this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_block = blockAUtiliser;

			this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(1, "."));
			this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(1, ".."));
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
		std::vector<std::string> pathElements = split(p_FileName, '/');
		std::string nomDuFichier = *pathElements.end()--;

		int blockLibre = bd_findFreeBlock();
		int inodeLibre = bd_findFreeInode();

		this->m_blockDisque.at(BASE_BLOCK_INODE + inodeLibre).m_inode->st_mode = S_IFDIR;

		this->m_blockDisque.at(blockLibre).m_dirEntry.push_back(new dirEntry(inodeLibre, nomDuFichier));

		return 0;
	}

	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation)
	{
		return "_";
	}

	int DisqueVirtuel::bd_rm(const std::string &p_Filename)
	{
		return 0;
	}

	int DisqueVirtuel::bd_findFreeBlock() const
	{
		int freeBlock = 0;

		for (int i = 0; i < this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.size(); i++)
		{
			if (this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(i))
			{
				freeBlock = i;
			}
		}

		return freeBlock;
	}

	std::vector<std::string> DisqueVirtuel::split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string number;

		while (std::getline(ss, number, delim))
		{
			elems.push_back(number);
		}

		return elems;
	}

	int DisqueVirtuel::bd_findFreeInode() const
	{
		int freeInode = 0;

		for (int i = 0; i < this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.size(); i++)
		{
			if (this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(i))
			{
				freeInode = i;
			}
		}

		return freeInode;
	}

	int DisqueVirtuel::doesParentExist(const std::string &p_DirName)
	{
		int succes = 0;

		int blockAUtiliser = bd_findFreeBlock();
		int inodeAUtiliser = bd_findFreeInode();

		std::vector<std::string> pathElements = split(p_DirName, '/');
		int blockAParcourir = m_blockRoot;
		for (auto repo = pathElements.begin(); repo < pathElements.end()--; repo++)
		{
			for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if(entry->m_filename == *repo)
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
				}
			}
		}

		/* this->m_blockDisque.at(BASE_BLOCK_INODE + inodeAUtiliser).m_inode->st_mode = S_IFDIR;
		this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(blockAUtiliser) = false;
		this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(inodeAUtiliser) = false;

		this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(inodeAUtiliser, "."));
		this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(inodeAUtiliser, "..")); */

		return succes;
	}

	DisqueVirtuel::~DisqueVirtuel() {}

} // Fin du namespace
