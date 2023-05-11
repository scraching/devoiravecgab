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
		std::cout << "Formattage" << std::endl;
		int succes = 1;

		try
		{
			std::vector<bool> initalisateurBlock(N_BLOCK_ON_DISK, true);
			// On marque les blocks 0 à 23 comme non-libres
			for (int i = 0; i < (N_INODE_ON_DISK + BASE_BLOCK_INODE); i++)
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

			for (auto i : m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap)
			{
				std::cout << "etat du bloc: " << i << std::endl;
			}

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

	//! TODO Adjust file size on concerned directories
	int DisqueVirtuel::bd_mkdir(const std::string &p_DirName)
	{
		std::vector<std::string> pathElements = split(p_DirName, '/');

		std::string nomDuFichier = pathElements.at(pathElements.size() - 1);

		int i = 0;
		int blockAParcourir = m_blockRoot;
		int inodeAParcourir;
		bool repoDecouvert = false;

		while (pathElements.at(i) != nomDuFichier)
		{
			for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_filename == pathElements.at(i))
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
					repoDecouvert = 1;
					inodeAParcourir = blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_ino;
				}
			}
			if (!repoDecouvert)
			{
				return 0;
			}

			i += 1;
		}
		if (!repoDecouvert)
		{
			return 0;
		}

		// On vérifie si le répertoire existe déjà
		for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
		{
			if (entry->m_filename == nomDuFichier)
			{
				return 0;
			}
		}

		int positionInode = bd_findFreeInode();
		int positionBlock = bd_findFreeBlock();

		std::cout << "UFS: Saisie bloc " << positionBlock << std::endl;
		std::cout << "UFS: Saisie i-node " << positionInode << std::endl;

		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_mode = S_IFDIR;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_nlink++;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_block = positionBlock;
		m_blockDisque.at(blockAParcourir).m_dirEntry.push_back(new dirEntry(positionInode + BASE_BLOCK_INODE, nomDuFichier));
		m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(positionBlock) = false;
		m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(positionInode) = false;
		m_blockDisque.at(positionBlock).m_dirEntry.push_back(new dirEntry(positionInode + BASE_BLOCK_INODE, "."));
		m_blockDisque.at(positionBlock).m_dirEntry.push_back(new dirEntry(inodeAParcourir, ".."));
		m_blockDisque.at(inodeAParcourir + BASE_BLOCK_INODE).m_inode->st_nlink++;

		return 1;
	}

	int DisqueVirtuel::bd_create(const std::string &p_FileName)
	{
		std::vector<std::string> pathElements = split(p_FileName, '/');
		std::string nomDuFichier = *(pathElements.end()--);

		/* this->m_blockDisque.at(BASE_BLOCK_INODE + inodeLibre).m_inode->st_mode = S_IFDIR;

		this->m_blockDisque.at(blockLibre).m_dirEntry.push_back(new dirEntry(inodeLibre, nomDuFichier)); */

		int i = 0;
		int blockAParcourir = m_blockRoot;
		bool repoDecouvert = false;
		while (pathElements.at(i) != nomDuFichier)
		{

			for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_filename == pathElements.at(i))
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
					repoDecouvert = 1;
				}
			}
			if (!repoDecouvert)
			{
				return 0;
			}

			i += 1;
		}
		if (!repoDecouvert)
		{
			return 0;
		}

		// On vérifie si le fichier existe déjà
		for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
		{
			if (entry->m_filename == nomDuFichier)
			{
				return 0;
			}
		}
		int positionInode = bd_findFreeInode();
		int positionBlock = bd_findFreeBlock();

		std::cout << "UFS: Saisie bloc " << positionBlock << std::endl;
		std::cout << "UFS: Saisie i-node " << positionInode << std::endl;

		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_mode = S_IFREG;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_nlink++;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_block = positionBlock;
		m_blockDisque.at(blockAParcourir).m_dirEntry.push_back(new dirEntry(positionInode + BASE_BLOCK_INODE, nomDuFichier));
		m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(positionBlock) = false;
		m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(positionInode) = false;
		return 1;
	}

	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation)
	{
		return "_";
	}

	int DisqueVirtuel::bd_rm(const std::string &p_Filename)
	{
		std::vector<std::string> pathElements = split(p_Filename, '/');
		std::string nomDuFichier = *(pathElements.end()--);

		int i = 0;
		int blockAParcourir = m_blockRoot;
		bool repoDecouvert = false;
		for (auto element : pathElements)
		{
			for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_filename == element)
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
					repoDecouvert = 1;
				}
			}
			if (!repoDecouvert)
			{
				return 0;
			}
		}
		if (!repoDecouvert)
		{
			return 0;
		}

		bool fileExists = false;
		int numeroInodeFichier;
		for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
		{
			if (entry->m_filename == nomDuFichier)
			{
				fileExists = true;
				numeroInodeFichier = entry->m_iNode;
				break;
			}
		}

		if (!fileExists)
		{
			return 0;
		}

		if(m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_mode == S_IFREG)
		{
			//
		}
		else
		{
			//
		}

		return 1;
	}

	int DisqueVirtuel::bd_findFreeBlock() const
	{
		int freeBlock = 0;

		for (int i = 0; i < this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.size(); i++)
		{
			if (this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(i))
			{
				freeBlock = i;
				break;
			}
		}

		std::cout << "Bloc libre: " << freeBlock << std::endl;

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
				break;
			}
		}

		std::cout << "i-node libre: " << freeInode << std::endl;

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
				if (entry->m_filename == *repo)
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
				}
			}
		}

		return succes;
	}

	DisqueVirtuel::~DisqueVirtuel() {}

} // Fin du namespace