/**
 * \file disqueVirtuel.h
 * \brief Gestion d'un disque virtuel.
 * \author GLO-2001
 * \version 0.1
 * \date  2022
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
#include <vector>

#ifndef _DISQUEVIRTUEL__H
#define _DISQUEVIRTUEL__H

namespace TP3
{

#define N_INODE_ON_DISK   20	// nombre maximal d'i-nodes (donc de fichiers) sur votre disque
#define N_BLOCK_ON_DISK 128	// nombre de blocs sur le disque au complet
#define FREE_BLOCK_BITMAP 2	// numero du bloc contenant le bitmap des block libres
#define FREE_INODE_BITMAP 3	// numero du bloc contenant le bitmap des i-nodes libres
#define BASE_BLOCK_INODE  4     // bloc de depart ou les i-nodes sont stockes sur disque
#define ROOT_INODE        1     // numero du i-node correspondant au repertoire racine

class DisqueVirtuel {
public:
	DisqueVirtuel();
	~DisqueVirtuel();

	// Méthodes demandées
	int bd_FormatDisk();
	std::string bd_ls(const std::string& p_DirLocation);
	int bd_mkdir(const std::string& p_DirName);
	int bd_create(const std::string& p_FileName);
	int bd_rm(const std::string& p_Filename);

	// Vous pouvez ajouter ici d'autres méthodes publiques

private:
	// Il est interdit de modifier ce modèle d'implémentation (i.e le type de m_blockDisque)!
    std::vector<Block> m_blockDisque; // Un vecteur de blocs représentant le disque virtuel
	int m_blockRoot;

    // Vous pouvez ajouter ici des méthodes privées
	int bd_findFreeBlock() const; // Parcourt la bitmap de blocks pour trouver le premier block libre
	int bd_findFreeInode() const; // PArcourt la bitmap d'i-nodes pour trouver la première i-node libre

	int doesParentExist(const std::string& p_DirName); // Vérifie si une string de répertoire parent est associée à une i-node

	std::vector<std::string> split(const std::string &s, char delim); // Sépare une string pour en récuṕerer les éléments séparés par delim
};

}//Fin du namespace

#endif
