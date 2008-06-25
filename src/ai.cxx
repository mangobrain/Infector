// Copyright 2008 Philip Allison <sane@not.co.uk>

//    This file is part of infector.
//
//    infector is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    infector is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with infector.  If not, see <http://www.gnu.org/licenses/>.


//
// Includes
//

// Standard
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

// Language headers
#include <memory>
#include <utility>
#include <vector>
#include <list>
#include <algorithm>
#include <bitset>
#include <queue>
#include <iostream>

// Library headers
#include <glibmm.h>
#include <sigc++/sigc++.h>

// Project headers
#include "boardstate.hxx"
#include "ai.hxx"
#include "game.hxx"


//
// Implementation
//

AI::AI(Game *game, const BoardState *bs)
	: m_pBoardState(bs), next_x(0), next_y(0)
{
	game->move_made.connect(sigc::mem_fun(*this, &AI::onMoveMade));

	// Construct root node of tree
	root = new treenode();
	root->parent = NULL;
	root->b.reset(new BoardState(*m_pBoardState));
	// Need to set this to the *last* player, so that children
	// will get player_1 as their player in the first round.
	// Doing this 3 times works for both 2 and 4 player games.
	root->b->nextPlayer();
	root->b->nextPlayer();
	root->b->nextPlayer();

	// Construct initial move tree
	std::list<treenode*> dummy;
	dummy.push_back(root);
	growTreeFromLeaves(dummy);
	growTreeFromLeaves(leaves);
	growTreeFromLeaves(leaves);
	//growTreeFromLeaves(leaves);
}

void AI::onMoveMade(const int start_x, const int start_y, const int end_x, const int end_y, const bool gameover)
{
	if (gameover)
		return;

	// Find the node corresponding to the move just made, and set it as the new root
	std::cout << "Move made: (" << start_x << ", " << start_y << "), (" << end_x << ", " << end_y << ")" << std::endl;
	treenode *oldroot = root;
	for (std::list<treenode*>::iterator i = root->children.begin(); i != root->children.end(); ++i)
	{
		if (start_x == (*i)->m.source_x && start_y == (*i)->m.source_y && end_x == (*i)->m.dest_x && end_y == (*i)->m.dest_y)
		{
			std::cout << "Found move!" << std::endl;
			root = *i;
			root->parent = NULL;
			break;
		}
	}
	
	// Delete all branches of the tree we're no longer interested in
	std::queue<treenode*> delqueue;
	delqueue.push(oldroot);
	std::cout << oldroot->children.size() << std::endl;
	oldroot->children.remove(root);
	std::cout << oldroot->children.size() << std::endl;
	/*while (!delqueue.empty())
	{
		treenode *n = delqueue.front();
		delqueue.pop();
		for (std::list<treenode*>::iterator i = n->children.begin(); i != n->children.end(); ++i)
			delqueue.push(*i);
		delete n;
	}*/
	delete oldroot;
	std::cout << "Done deleting" << std::endl;
	
	// Rebuild the leaf list
	leaves.clear();
	std::queue<treenode*> leafqueue;
	leafqueue.push(root);
	while (!leafqueue.empty())
	{
		treenode *n = leafqueue.front();
		leafqueue.pop();
		if (n->children.empty())
			leaves.push_back(n);
		else
		{
			for (std::list<treenode*>::iterator i = n->children.begin(); i != n->children.end(); ++i)
				leafqueue.push(*i);
		}
	}

	// Keep move cache updated with new leaf nodes
	growTreeFromLeaves(leaves);

	if (m_pBoardState->isAIPlayer(m_pBoardState->getPlayer()))
	{
		std::vector<treenode*> nodes(root->children.begin(), root->children.end());
		std::random_shuffle(nodes.begin(), nodes.end());
		// Highlight the square we're going to move
		square_clicked(nodes.front()->m.source_x, nodes.front()->m.source_y);
		next_x = nodes.front()->m.dest_x;
		next_y = nodes.front()->m.dest_y;
		// Make the actual move in 0.5 seconds time (to let people see)
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &AI::makeMove), 500);
	}
}

bool AI::makeMove() const
{
	square_clicked(next_x, next_y);
	// Don't keep firing the timer after this call
	return false;
}

// Expand the tree by generating new leaf nodes from the given set
// of leaf nodes.  Pass list in by value in case it's our own leaf set,
// because we overwrite the latter.
void AI::growTreeFromLeaves(std::list<treenode*> l)
{
	leaves.clear();
	for (std::list<treenode*>::iterator currnode = l.begin(); currnode != l.end(); ++currnode)
	{
		// Determine whose turn it is after the current node's move
		// Take a copy of the current board state for this node
		BoardState *b = new BoardState(*((*currnode)->b));
		piece orig_p = b->getPlayer();
		piece p = b->nextPlayer();
		bool gameover = false;
		while (!(b->canMove(p)))
		{
			p = b->nextPlayer();
			if (p == orig_p)
			{
				gameover = true;
				break;
			}
		}
		// nobody can move - no subsequent moves to generate.
		if (gameover)
		{
			delete b;
			continue;
		}

		unsigned int depth = 1;
		treenode *t = *currnode;
		while (t->parent != NULL)
		{
			++depth;
			t = t->parent;
			if (depth > 3)
				break;
		}

		std::vector<move> moves((*currnode)->b->getPossibleMoves(p));
		for (std::vector<move>::iterator i = moves.begin(); i != moves.end(); ++i)
		{
			treenode *node = new treenode();
			node->m = *i;
			
			// Simulate our move
			node->b.reset(b);
			int adj = node->b->getAdjacency(node->m.source_x, node->m.source_y, node->m.dest_x, node->m.dest_y);
			if (adj == 2)
				node->b->setPieceAt(node->m.source_x, node->m.source_y, player_none);
			node->b->setPieceAt(node->m.dest_x, node->m.dest_y, p);
			
			// Use heuristics to score the move
			// Prefer cloning to jumping
			node->score = (3 - adj) * 2;
			// Capture enemy pieces
			for (int xx = node->m.dest_x - 1; xx <= node->m.dest_x + 1; ++xx)
			{
				for (int yy = node->m.dest_y - 1; yy <= node->m.dest_y + 1; ++yy)
				{
					// BoardState does range checking for us
					piece capturesquare = node->b->getPieceAt(xx, yy);
					if (capturesquare == no_such_square)
						continue;
					if (capturesquare == player_none)
					{
						// Enjoy exploring - a little
						node->score += 1;
						continue;
					}
					if (capturesquare == p)
					{
						// Enjoy defending
						node->score += 2;
						continue;
					}
					node->b->setPieceAt(node->m.dest_x, node->m.dest_y, p);
					// Enjoy capturing enemy pieces the most
					node->score += 3;
				}
			}
			
			// If we've rendered any opponents immobile, score big points.
			// If we ourselves are immobile - this may not be a good move.
			// Don't worry that we may only have a 2-player game - just means
			// every move gets 20 added here, so it evens out.
			if (!(node->b->canMove(player_1)))
				node->score += 10 * ((p == player_1) ? -1 : 1);
			if (!(node->b->canMove(player_2)))
				node->score += 10 * ((p == player_2) ? -1 : 1);
			if (!(node->b->canMove(player_3)))
				node->score += 10 * ((p == player_3) ? -1 : 1);
			if (!(node->b->canMove(player_4)))
				node->score += 10 * ((p == player_4) ? -1 : 1);
			
			// Add ourselves to the tree
			node->parent = *currnode;
			(*currnode)->children.push_back(node);
			
			// If current depth is just right, add ourselves to the leaf node list
			if (depth <= 3)
				leaves.push_back(node);
		}
	}
}
