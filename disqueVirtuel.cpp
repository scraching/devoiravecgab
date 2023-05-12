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

		// Création des i-nodes
		for (int i = BASE_BLOCK_INODE; i < (N_INODE_ON_DISK + BASE_BLOCK_INODE); i++)
		{
			this->m_blockDisque.at(i).m_inode = new iNode(i - BASE_BLOCK_INODE, 0, 0, 0, 0);
		}
	}

	int DisqueVirtuel::bd_FormatDisk()
	{
		int succes = 1;

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

		for (auto block : this->m_blockDisque)
		{
			if (block.m_dirEntry.size() > 0)
			{
				int tailleDirEntry = block.m_dirEntry.size();
				for (auto entry : block.m_dirEntry)
				{
					delete entry;
				}
				block.m_dirEntry.clear();
			}
		}

		for (int i = BASE_BLOCK_INODE; i < (N_INODE_ON_DISK + BASE_BLOCK_INODE); i++)
		{
			this->m_blockDisque.at(i).m_inode->st_ino = i - BASE_BLOCK_INODE;
			this->m_blockDisque.at(i).m_inode->st_mode = 0;
			this->m_blockDisque.at(i).m_inode->st_nlink = 0;
			this->m_blockDisque.at(i).m_inode->st_size = 0;
			this->m_blockDisque.at(i).m_inode->st_block = 0;
		}

		// Création du répertoire racine
		int blockAUtiliser = bd_findFreeBlock();
		m_blockRoot = blockAUtiliser;

		this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(blockAUtiliser) = false;
		this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(ROOT_INODE) = false;

		this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_mode = S_IFDIR;
		this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_nlink = 2;
		this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_size = 2 * 28;
		this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_block = blockAUtiliser;

		this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(ROOT_INODE, "."));
		this->m_blockDisque.at(blockAUtiliser).m_dirEntry.push_back(new dirEntry(ROOT_INODE, ".."));

		// Le formattage échoue si la taille du disque virtuel formatté est incorrecte.
		if (m_blockDisque.size() != N_BLOCK_ON_DISK)
		{
			succes = 0;
		}

		std::cout << "UFS: Saisit bloc " << m_blockRoot << std::endl;
		std::cout << "UFS: Saisit i-node " << ROOT_INODE << std::endl;

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

		bool repoDecouvert = pathElements.size() >= 3 ? false : true; // Créer un répertoire à partir de / est un cas spécial

		if (pathElements.size() <= 2)
		{
			inodeAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + 1).m_inode->st_ino;
		}
		else
		{
			while (pathElements.at(i) != nomDuFichier)
			{
				for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
				{
					if (entry->m_filename == pathElements.at(i) && this->m_blockDisque.at(BASE_BLOCK_INODE + entry->m_iNode).m_inode->st_mode == S_IFDIR)
					{
						int inodeNouveauBlock = entry->m_iNode;
						blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
						repoDecouvert = true;
						inodeAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_ino;
					}
				}

				i += 1;
			}
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

		// On trouve le prochain bloc libre et la prochaine i-node libres pour la crátion du répertoire
		int positionInode = bd_findFreeInode();
		int positionBlock = bd_findFreeBlock();

		if (positionBlock == 0 || positionInode == 0)
		{
			return 0;
		}

		std::cout << "UFS: Saisie bloc " << positionBlock << std::endl;
		std::cout << "UFS: Saisie i-node " << positionInode << std::endl;

		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_mode = S_IFDIR;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_nlink++;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_block = positionBlock;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_size += 28;
		m_blockDisque.at(blockAParcourir).m_dirEntry.push_back(new dirEntry(positionInode, nomDuFichier));

		m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(positionBlock) = false;
		m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(positionInode) = false;

		m_blockDisque.at(positionBlock).m_dirEntry.push_back(new dirEntry(positionInode + BASE_BLOCK_INODE, "."));
		m_blockDisque.at(positionBlock).m_dirEntry.push_back(new dirEntry(inodeAParcourir, ".."));
		m_blockDisque.at(inodeAParcourir + BASE_BLOCK_INODE).m_inode->st_nlink++;
		m_blockDisque.at(inodeAParcourir + BASE_BLOCK_INODE).m_inode->st_size += 2 * 28;

		return 1;
	}

	int DisqueVirtuel::bd_create(const std::string &p_FileName)
	{
		std::vector<std::string> pathElements = split(p_FileName, '/');
		std::string nomDuFichier = pathElements.at(pathElements.size() - 1);

		int i = 0;
		int blockAParcourir = m_blockRoot;
		bool repoDecouvert = pathElements.size() >= 3 ? false : true; // Créer un fichier à partir de / est un cas spécial

		if (pathElements.size() >= 3)
		{
			while (pathElements.at(i) != nomDuFichier)
			{
				for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
				{
					if (entry->m_filename == pathElements.at(i) && this->m_blockDisque.at(BASE_BLOCK_INODE + entry->m_iNode).m_inode->st_mode == S_IFDIR)
					{
						int inodeNouveauBlock = entry->m_iNode;
						blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
						repoDecouvert = true;
					}
				}

				i += 1;
			}
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

		// On trouve le prochain bloc libre et la prochaine i-node libres pour la création du fichier
		int positionInode = bd_findFreeInode();
		int positionBlock = bd_findFreeBlock();

		if (positionBlock == 0 || positionInode == 0)
		{
			return 0;
		}

		std::cout << "UFS: Saisie bloc " << positionBlock << std::endl;
		std::cout << "UFS: Saisie i-node " << positionInode << std::endl;

		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_mode = S_IFREG;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_nlink++;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_block = positionBlock;
		m_blockDisque.at(positionInode + BASE_BLOCK_INODE).m_inode->st_size += 28;
		m_blockDisque.at(blockAParcourir).m_dirEntry.push_back(new dirEntry(positionInode, nomDuFichier));
		m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(positionBlock) = false;
		m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(positionInode) = false;

		return 1;
	}

	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation)
	{
		std::ostringstream output;

		output << p_DirLocation << "\n";

		std::vector<std::string> pathElements = split(p_DirLocation, '/');
		std::string fileName = pathElements.at(pathElements.size() - 1);

		int blockAParcourir = m_blockRoot;
		for (int i = 0; i < pathElements.size(); i++)
		{
			for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_filename == pathElements.at(i) && this->m_blockDisque.at(BASE_BLOCK_INODE + entry->m_iNode).m_inode->st_mode == S_IFDIR)
				{
					int inodeNouveauBlock = entry->m_iNode;
					blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
				}
			}
		}

		// Boucle qui formatte l'affichage de chaque entrée du répertoire
		for (auto entry : this->m_blockDisque.at(blockAParcourir).m_dirEntry)
		{
			std::ostringstream nomRepo;
			int longueurRepo = 1;
			std::string nomRepoString = entry->m_filename;
			if (this->m_blockDisque.at(entry->m_iNode + BASE_BLOCK_INODE).m_inode->st_mode == S_IFREG)
			{
				nomRepo << "-";
			}
			else
			{
				nomRepo << "d";
			}
			while (longueurRepo < 16 - nomRepoString.size())
			{
				nomRepo << " ";
				longueurRepo++;
			}
			nomRepo << nomRepoString;
			output << nomRepo.str();
			output << " ";

			std::ostringstream tailleRepo;
			tailleRepo << "Size: ";
			int longueurTaille = 5;
			std::string tailleRepoString = std::to_string(this->m_blockDisque.at(entry->m_iNode + BASE_BLOCK_INODE).m_inode->st_size);
			while (longueurTaille < 11 - tailleRepoString.size())
			{
				tailleRepo << " ";
				longueurTaille++;
			}
			output << tailleRepo.str();
			output << tailleRepoString;
			output << " ";

			std::ostringstream numeroInode;
			numeroInode << "inode: ";
			int longueurInode = 6;
			std::string numeroInodeString = std::to_string(this->m_blockDisque.at(entry->m_iNode + BASE_BLOCK_INODE).m_inode->st_ino);
			while (longueurInode < 10 - numeroInodeString.size())
			{
				numeroInode << " ";
				longueurInode++;
			}
			output << numeroInode.str();
			output << numeroInodeString;
			output << " ";

			std::ostringstream nombreLiens;
			nombreLiens << "nlink: ";
			int longueurLiens = 6;
			std::string nombreLiensString = std::to_string(this->m_blockDisque.at(entry->m_iNode + BASE_BLOCK_INODE).m_inode->st_nlink);
			while (longueurLiens < 10 - nombreLiensString.size())
			{
				nombreLiens << " ";
				longueurLiens++;
			}
			output << nombreLiens.str();
			output << nombreLiensString;
			output << "\n";
		}

		return output.str();
	}

	int DisqueVirtuel::bd_rm(const std::string &p_Filename)
	{
		std::vector<std::string> pathElements = split(p_Filename, '/');
		std::string nomDuFichier = pathElements.at(pathElements.size() - 1);

		int i = 0;
		int blockAParcourir = m_blockRoot;
		bool repoDecouvert = pathElements.size() >= 3 ? false : true;

		if (pathElements.size() >= 3)
		{
			while (pathElements.at(i) != nomDuFichier)
			{
				for (auto entry : m_blockDisque.at(blockAParcourir).m_dirEntry)
				{
					if (entry->m_filename == pathElements.at(i) && this->m_blockDisque.at(BASE_BLOCK_INODE + entry->m_iNode).m_inode->st_mode == S_IFDIR)
					{
						int inodeNouveauBlock = entry->m_iNode;
						blockAParcourir = this->m_blockDisque.at(BASE_BLOCK_INODE + inodeNouveauBlock).m_inode->st_block;
						repoDecouvert = true;
					}
				}
				i++;
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

		if (this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_mode == S_IFREG)
		{
			this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_nlink--;
			this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_mode = 0;
			// this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_block = 0;

			int positionEntreeASupprimer = 0;
			for (auto entry : this->m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_iNode == numeroInodeFichier)
				{
					break;
				}
				positionEntreeASupprimer++;
			}
			delete this->m_blockDisque.at(blockAParcourir).m_dirEntry.at(positionEntreeASupprimer);
			this->m_blockDisque.at(blockAParcourir).m_dirEntry.erase(this->m_blockDisque.at(blockAParcourir).m_dirEntry.begin() + positionEntreeASupprimer);

			int blockALibrer = this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_block;

			this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(blockALibrer) = true;
			std::cout << "UFS: Relache bloc " << blockALibrer << std::endl;

			if (this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_nlink == 0)
			{
				this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(numeroInodeFichier) = true;
				std::cout << "UFS: Relache i-node " << numeroInodeFichier << std::endl;
			}
		}
		else
		{
			int inodeALiberer;
			int blockALiberer;
			for (auto entry : this->m_blockDisque.at(blockAParcourir).m_dirEntry)
			{
				if (entry->m_filename == nomDuFichier)
				{
					inodeALiberer = entry->m_iNode;
					blockALiberer = this->m_blockDisque.at(inodeALiberer + BASE_BLOCK_INODE).m_inode->st_block;
				}
			}

			if (this->m_blockDisque.at(blockALiberer).m_dirEntry.size() <= 2)
			{
				delete this->m_blockDisque.at(blockALiberer).m_dirEntry.at(0);
				delete this->m_blockDisque.at(blockALiberer).m_dirEntry.at(1);
				this->m_blockDisque.at(blockALiberer).m_dirEntry.pop_back();
				this->m_blockDisque.at(blockALiberer).m_dirEntry.pop_back();

				this->m_blockDisque.at(FREE_BLOCK_BITMAP).m_bitmap.at(blockALiberer) = true;
				std::cout << "UFS: Relache bloc " << blockALiberer << std::endl;

				this->m_blockDisque.at(inodeALiberer + BASE_BLOCK_INODE).m_inode->st_nlink--;
				this->m_blockDisque.at(inodeALiberer + BASE_BLOCK_INODE).m_inode->st_size -= 28;
				this->m_blockDisque.at(inodeALiberer + BASE_BLOCK_INODE).m_inode->st_mode = 0;
				this->m_blockDisque.at(inodeALiberer + BASE_BLOCK_INODE).m_inode->st_block = 0;
				this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(inodeALiberer) = true;

				int positionEntreeASupprimer = 0;
				for (auto entry : this->m_blockDisque.at(blockAParcourir).m_dirEntry)
				{
					if (entry->m_iNode == numeroInodeFichier)
					{
						break;
					}
					positionEntreeASupprimer++;
				}
				delete this->m_blockDisque.at(blockAParcourir).m_dirEntry.at(positionEntreeASupprimer);
				this->m_blockDisque.at(blockAParcourir).m_dirEntry.erase(this->m_blockDisque.at(blockAParcourir).m_dirEntry.begin() + positionEntreeASupprimer);

				if (this->m_blockDisque.at(numeroInodeFichier + BASE_BLOCK_INODE).m_inode->st_nlink == 0)
				{
					this->m_blockDisque.at(FREE_INODE_BITMAP).m_bitmap.at(numeroInodeFichier) = true;
					std::cout << "UFS: Relache i-node " << numeroInodeFichier << std::endl;
				}
			}
			else
			{
				return 0;
			}
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