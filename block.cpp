/**
 * \file block.cpp
 * \brief Implémentation d'un bloc.
 * \author ?
 * \version 0.1
 * \date  2022
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
// vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{
	// Ajouter votre code ici !
	Block::Block() {}

	Block::Block(size_t td)
	{
		this->m_type_donnees = td;
	}
}

// Fin du namespace
