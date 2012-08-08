
#include <thread>
#include <fstream>
#include <algorithm>
#include "Entry.h"

using namespace std;

int SEPARADOR = 128;
int GRUPOS_HILO = 1024;
int NUM_THREADS = 4;
int TOPE = 1023;
int MAX_VECINOS = 64;
int MAX_RAILS = 16;

unsigned char const MOVE_UP = 0;
unsigned char const MOVE_U_R = 1;
unsigned char const MOVE_RIGHT = 2;
unsigned char const MOVE_D_R = 3;
unsigned char const MOVE_DOWN = 4;
unsigned char const MOVE_D_L = 5;
unsigned char const MOVE_LEFT = 6;
unsigned char const MOVE_U_L = 7;

std::vector<uint16_t> map;
std::vector<unsigned char> neighbours_number;
std::vector<uint16_t> neighbours_groupID;
std::vector<uint16_t> group_distances;
std::vector<uint16_t> group_refloc;
std::vector<unsigned char> rails;

std::vector<uint16_t> visited;

int basic_width;
int basic_height;

std::vector<xyLoc> succ;

xyLoc actual_point;

uint16_t id_grupo_actual;
uint16_t id_grupo_meta;
uint16_t id_last_grupo;

bool railes_ultimo;

int SacarGrupoInsignia(std::vector<int>* grupoauxiliar_grupo, int id_grupo);

void GetSuccessors(xyLoc s, std::vector<xyLoc> &neighbors);
void GetSuccessors_cardinals(xyLoc s, std::vector<xyLoc> &neighbors);
void GetSuccessors_diagonals(xyLoc s, std::vector<xyLoc> &neighbors);
void ExtractPath(xyLoc end, std::vector<xyLoc> &finalPath);
void ExtractPathSearchPathToGroup(xyLoc end, std::vector<xyLoc> &finalPath);
int GetIndex(xyLoc s);

const char *GetName(){
	return "bubble-dragon";
}

unsigned char GetMovement(xyLoc origin, xyLoc next){
	unsigned char movement;
	if(origin.y-1 == next.y && origin.x == next.x){		
		movement = MOVE_UP;

	}else if(origin.y-1 == next.y && origin.x+1 == next.x){
		movement = MOVE_U_R;

	}else if(origin.y == next.y   && origin.x+1 == next.x){
		movement = MOVE_RIGHT;

	}else if(origin.y+1 == next.y && origin.x+1 == next.x){
		movement = MOVE_D_R;

	}else if(origin.y+1 == next.y && origin.x == next.x){
		movement = MOVE_DOWN;

	}else if(origin.y+1 == next.y && origin.x-1 == next.x){
		movement = MOVE_D_L;

	}else if(origin.y == next.y   && origin.x-1 == next.x){
		movement = MOVE_LEFT;

	}else if(origin.y-1 == next.y && origin.x-1 == next.x){
		movement = MOVE_U_L;
	}
	return movement;
}

// Normal Load Rails

void WriteRails_1(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){

	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;

	if(actual_neighbour == 0){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 6 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}
				
			if(step_it != steps.end()){
				m_counter = 1;
				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}				
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}		
	}
}
void WriteRails_2(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){

	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;
	
	if(actual_neighbour == 0){

		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 3 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;
				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());		

				rails[(point.y*basic_width + point.x)*6 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 1) {
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 3 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;
				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());			
				rails[(point.y*basic_width + point.x)*6 + i + 3] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}
}
void WriteRails_3(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){

	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;

	if(actual_neighbour == 0){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 1) {
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + 2 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 2){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){				
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + 4 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}
}
void WriteRails_4(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){

	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;

	if(actual_neighbour == 0){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 1) {
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + 2 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 2){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

		rails[(point.y*basic_width + point.x)*6 + 4] = m_now*32 + m_counter;

	}else if(actual_neighbour == 3){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));
		
		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());		

		rails[(point.y*basic_width + point.x)*6 + 5] = m_now*32 + m_counter;
	}
}
void WriteRails_5(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;

	if(actual_neighbour == 0){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 1;

		for(int i = 0; i < 2 && step_it != steps.end(); i++){

			if(m_counter == 31){
				origin = (*step_it);
				step_it++;
				if(step_it != steps.end()){
					m_now = GetMovement(origin, (*step_it));
				}
			}

			if(step_it != steps.end()){
				m_counter = 1;

				do{
					origin = (*step_it);
					step_it++;
					if(step_it != steps.end()){
						m_next = GetMovement(origin, (*step_it));
						if(m_now == m_next){
							m_counter++;
						}
					}
				}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

				rails[(point.y*basic_width + point.x)*6 + i] = m_now*32 + m_counter;
				m_now = m_next;
			}
		}
	}else if(actual_neighbour == 1) {
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

		rails[(point.y*basic_width + point.x)*6 + 2] = m_now*32 + m_counter;		
	}else if(actual_neighbour == 2){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

		rails[(point.y*basic_width + point.x)*6 + 3] = m_now*32 + m_counter;		
	}else if(actual_neighbour == 3){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

		rails[(point.y*basic_width + point.x)*6 + 4] = m_now*32 + m_counter;		
	}else if(actual_neighbour == 4){
		step_it = steps.begin();
		xyLoc origin = point;
		m_now = GetMovement(origin, (*step_it));

		m_counter = 0;

		do{
			origin = (*step_it);
			step_it++;
			if(step_it != steps.end()){
				m_next = GetMovement(origin, (*step_it));
				if(m_now == m_next){
					m_counter++;
				}
			}
		}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

		rails[(point.y*basic_width + point.x)*6 + 5] = m_now*32 + m_counter;		
	}
}
void WriteRails_6(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 31;		
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour < 6){

		rails[(point.y*basic_width + point.x)*6 + actual_neighbour] = m_now*32 + m_counter;
	}
}
void WriteRails_7(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 15;
	if(actual_neighbour > 5){
		m_counter_limit = 7;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*2;		

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += (m_now/4);	
		rails[(point.y*basic_width + point.x)*6 + 1] += (m_now%4)*64 + m_counter*4;	

	}else if(actual_neighbour == 2){
	
		rails[(point.y*basic_width + point.x)*6 + 1] += (m_now/2);
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now%2)*128 + m_counter*8;	

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now);
		rails[(point.y*basic_width + point.x)*6 + 3] += m_counter*16;

	}else if(actual_neighbour == 4){
		
		rails[(point.y*basic_width + point.x)*6 + 3] += (m_now)*2 + m_counter/8;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_counter%8)*32;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now)*4 + m_counter/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_counter%4)*64;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now)*8 + m_counter;
	
	}
}
void WriteRails_8(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 7;

	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*4;		

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 1] += (m_now%2)*128 + m_counter*16;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2 + m_counter/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_counter%4)*64;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*8 + m_counter;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32 + m_counter*4;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now%2)*128 + m_counter*16;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2 + m_counter/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_counter%4)*64;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*8 + m_counter;
	
	}
}
void WriteRails_9(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 7;
	if(actual_neighbour > 2){
		m_counter_limit = 3;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*4;		

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 1] += (m_now%2)*128 + m_counter*16;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2 + m_counter/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_counter%4)*64;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*8 + m_counter*2;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 3] += (m_now%4)*64 + m_counter*16;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*2 + m_counter/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_counter%2)*128;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*16 + m_counter*4;		

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now%2)*128 + m_counter*32;
	
	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*4 + m_counter;
	
	}
}
void WriteRails_10(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 3;
	if(actual_neighbour > 7){
		m_counter_limit = 1;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*8;		

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now;
		rails[(point.y*basic_width + point.x)*6 + 1] += m_counter*64;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*8 + m_counter*2;		

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now%4)*64 + m_counter*16;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*2 + m_counter/2;
		rails[(point.y*basic_width + point.x)*6 + 3] += (m_counter%2)*128;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*16 + m_counter*4;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now%2)*128 + m_counter*32;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*4 + m_counter;
	
	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*2 + m_counter;
	
	}
}
void WriteRails_11(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 3;
	if(actual_neighbour > 3){
		m_counter_limit = 1;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	do{
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}while(m_next == m_now && m_counter<m_counter_limit && step_it != steps.end());

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*8;		

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now;
		rails[(point.y*basic_width + point.x)*6 + 1] += m_counter*64;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*8 + m_counter*2;		

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now%4)*64 + m_counter*16;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*2 + m_counter;	

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*2 + m_counter;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*32 + m_counter*16;
	
	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2 + m_counter;

	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 10){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*2 + m_counter;

	}
}
void WriteRails_12(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;

	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	origin = (*step_it);
	step_it++;
	if(step_it != steps.end()){
		m_next = GetMovement(origin, (*step_it));
		if(m_now == m_next){
			m_counter++;
		}
	}

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now*2 + m_counter;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2 + m_counter;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*2 + m_counter;	

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*2 + m_counter;

	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*32 + m_counter*16;
	
	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2 + m_counter;

	}else if(actual_neighbour == 10){

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 11){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*2 + m_counter;
	
	}
}
void WriteRails_13(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 1;
	if(actual_neighbour > 8){
		m_counter_limit = 0;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	if(m_counter_limit > 0){
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now*2 + m_counter;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2 + m_counter;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*2 + m_counter;	

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*2 + m_counter;

	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*32 + m_counter*16;
	
	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2;

	}else if(actual_neighbour == 10){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now%4)*64;

	}else if(actual_neighbour == 11){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*8;
	
	}else if(actual_neighbour == 12){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now;

	}
}
void WriteRails_14(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 1;
	if(actual_neighbour > 5){
		m_counter_limit = 0;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	if(m_counter_limit > 0){
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now*2 + m_counter;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2 + m_counter;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*2 + m_counter;	

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*4;

	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now%2)*128;
	
	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*16;

	}else if(actual_neighbour == 10){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2;

	}else if(actual_neighbour == 11){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now%4)*64;

	}else if(actual_neighbour == 12){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*8;
	
	}else if(actual_neighbour == 13){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now;

	}
}
void WriteRails_15(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;
	unsigned char m_next;
	unsigned char m_counter;
	unsigned char m_counter_limit = 1;
	if(actual_neighbour > 2){
		m_counter_limit = 0;
	}
	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	m_counter = 0;

	if(m_counter_limit > 0){
		origin = (*step_it);
		step_it++;
		if(step_it != steps.end()){
			m_next = GetMovement(origin, (*step_it));
			if(m_now == m_next){
				m_counter++;
			}
		}
	}

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now*2 + m_counter;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*32 + m_counter*16;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2;

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now%4)*64;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*8;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32;

	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*4;

	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now%2)*128;
	
	}else if(actual_neighbour == 10){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*16;

	}else if(actual_neighbour == 11){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2;

	}else if(actual_neighbour == 12){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now%4)*64;

	}else if(actual_neighbour == 13){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*8;
	
	}else if(actual_neighbour == 14){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now;

	}
}
void WriteRails_16(int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){	
	unsigned char m_now;

	std::deque<xyLoc>::iterator step_it;

	step_it = steps.begin();
	xyLoc origin = point;
	m_now = GetMovement(origin, (*step_it));

	if(actual_neighbour == 0){

		rails[(point.y*basic_width + point.x)*6] += m_now*32;

	}else if(actual_neighbour == 1) {

		rails[(point.y*basic_width + point.x)*6] += m_now*4;

	}else if(actual_neighbour == 2){

		rails[(point.y*basic_width + point.x)*6] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 1] += (m_now%2)*128;

	}else if(actual_neighbour == 3){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*16;		

	}else if(actual_neighbour == 4){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now*2;

	}else if(actual_neighbour == 5){

		rails[(point.y*basic_width + point.x)*6 + 1] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 2] += (m_now%4)*64;

	}else if(actual_neighbour == 6){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now*8;

	}else if(actual_neighbour == 7){

		rails[(point.y*basic_width + point.x)*6 + 2] += m_now;

	}else if(actual_neighbour == 8){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*32;

	}else if(actual_neighbour == 9){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now*4;

	}else if(actual_neighbour == 10){

		rails[(point.y*basic_width + point.x)*6 + 3] += m_now/2;
		rails[(point.y*basic_width + point.x)*6 + 4] += (m_now%2)*128;
	
	}else if(actual_neighbour == 11){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*16;

	}else if(actual_neighbour == 12){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now*2;

	}else if(actual_neighbour == 13){

		rails[(point.y*basic_width + point.x)*6 + 4] += m_now/4;
		rails[(point.y*basic_width + point.x)*6 + 5] += (m_now%4)*64;

	}else if(actual_neighbour == 14){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now*8;
	
	}else if(actual_neighbour == 15){	

		rails[(point.y*basic_width + point.x)*6 + 5] += m_now;

	}
}
void WriteRails(int total_neighbours, int actual_neighbour, xyLoc point, std::deque<xyLoc> & steps){
	
	switch(total_neighbours){
		case 1:
			WriteRails_1(actual_neighbour, point, steps);
			break;
		case 2:
			WriteRails_2(actual_neighbour, point, steps);
			break;
		case 3:
			WriteRails_3(actual_neighbour, point, steps);
			break;
		case 4:
			WriteRails_4(actual_neighbour, point, steps);
			break;
		case 5:
			WriteRails_5(actual_neighbour, point, steps);
			break;
		case 6:
			WriteRails_6(actual_neighbour, point, steps);
			break;
		case 7:
			WriteRails_7(actual_neighbour, point, steps);
			break;
		case 8:
			WriteRails_8(actual_neighbour, point, steps);
			break;
		case 9:
			WriteRails_9(actual_neighbour, point, steps);
			break;
		case 10:
			WriteRails_10(actual_neighbour, point, steps);
			break;
		case 11:
			WriteRails_11(actual_neighbour, point, steps);
			break;
		case 12:
			WriteRails_12(actual_neighbour, point, steps);
			break;
		case 13:
			WriteRails_13(actual_neighbour, point, steps);
			break;
		case 14:
			WriteRails_14(actual_neighbour, point, steps);
			break;
		case 15:
			WriteRails_15(actual_neighbour, point, steps);
			break;
		default:
			WriteRails_16(actual_neighbour, point, steps);
			break;		
	}
}

// Read Rails

xyLoc GetNextLoc(xyLoc origin, unsigned char movement){

	switch(movement){
		case MOVE_UP:
			origin.y--;
			break;
		case MOVE_U_R:
			origin.y--;
			origin.x++;
			break;
		case MOVE_RIGHT:
			origin.x++;
			break;
		case MOVE_D_R:
			origin.y++;
			origin.x++;			
			break;
		case MOVE_DOWN:
			origin.y++;
			break;
		case MOVE_D_L:
			origin.y++;
			origin.x--;
			break;
		case MOVE_LEFT:
			origin.x--;
			break;
		case MOVE_U_L:
			origin.y--;
			origin.x--;			
			break;			
	}
	return origin;
}

void ReadRails_1(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){
	
	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;

	if(actual_neighbour == 0){
		longitud = 1;
		for(int i = 0; i<6 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}
	}
}
void ReadRails_2(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;	

	if(actual_neighbour == 0){		
		longitud = 1;
		for(int i = 0; i<3 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i]%32;
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}		
	}else if(actual_neighbour == 1){		
		longitud = 1;
		for(int i = 0; i<3 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i + 3]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i + 3]%32;						
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}		
	}
}
void ReadRails_3(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}

	}else if(actual_neighbour == 1){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i + 2]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i + 2]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}

	}else if(actual_neighbour == 2){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i + 4]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i + 4]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}
	}
}
void ReadRails_4(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}

	}else if(actual_neighbour == 1){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i + 2]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i + 2]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}

	}else if(actual_neighbour == 2){
		
		movement = rails[(point.y*basic_width + point.x)*6 + 4]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 4]%32;
		
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);
			steps.push_back(nuevo_punto);			
		}		

	}else if(actual_neighbour == 3){

		movement = rails[(point.y*basic_width + point.x)*6 + 5]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 5]%32;		
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);
			steps.push_back(nuevo_punto);			
		}			
	}
}
void ReadRails_5(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){
		longitud = 1;
		for(int i = 0; i<2 && longitud > 0; i++){
			movement = rails[(point.y*basic_width + point.x)*6 + i]/32;
			longitud = rails[(point.y*basic_width + point.x)*6 + i]%32;
			
			for(int j = 0 ; j < longitud; j++){
				nuevo_punto = GetNextLoc(nuevo_punto, movement);
				steps.push_back(nuevo_punto);			
			}			
		}

	}else if(actual_neighbour == 1){
	
		movement = rails[(point.y*basic_width + point.x)*6 + 2]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 2]%32;
		
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);
			steps.push_back(nuevo_punto);			
		}			

	}else if(actual_neighbour == 2){
		
		movement = rails[(point.y*basic_width + point.x)*6 + 3]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 3]%32;
		
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);
			steps.push_back(nuevo_punto);			
		}		

	}else if(actual_neighbour == 3){

		movement = rails[(point.y*basic_width + point.x)*6 + 4]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 4]%32;
			
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);
			steps.push_back(nuevo_punto);			
		}			

	}else if(actual_neighbour == 4){

		movement = rails[(point.y*basic_width + point.x)*6 + 5]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 5]%32;
			
		for(int j = 0 ; j <= longitud; j++){
			nuevo_punto = GetNextLoc(nuevo_punto, movement);		
			steps.push_back(nuevo_punto);			
		}			
	}
}
void ReadRails_6(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = rails[(point.y*basic_width + point.x)*6]/32;
		longitud = rails[(point.y*basic_width + point.x)*6]%32;							

	}else if(actual_neighbour == 1){
	
		movement = rails[(point.y*basic_width + point.x)*6 + 1]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 1]%32;	

	}else if(actual_neighbour == 2){
		
		movement = rails[(point.y*basic_width + point.x)*6 + 2]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 2]%32;		

	}else if(actual_neighbour == 3){

		movement = rails[(point.y*basic_width + point.x)*6 + 3]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 3]%32;		

	}else if(actual_neighbour == 4){

		movement = rails[(point.y*basic_width + point.x)*6 + 4]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 4]%32;

	}else if(actual_neighbour == 5){

		movement = rails[(point.y*basic_width + point.x)*6 + 5]/32;
		longitud = rails[(point.y*basic_width + point.x)*6 + 5]%32;					

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_7(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = rails[(point.y*basic_width + point.x)*6]/32;
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/2;

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 1]/64);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%64)/4;

	}else if(actual_neighbour == 2){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 2]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%128)/8;

	}else if(actual_neighbour == 3){

		movement = rails[(point.y*basic_width + point.x)*6 + 2]%8;
		longitud = rails[(point.y*basic_width + point.x)*6 + 3]/16;	

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%2)*8 + (rails[(point.y*basic_width + point.x)*6 + 4]/32);

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%32)/4;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%4)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%8);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_8(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = rails[(point.y*basic_width + point.x)*6]/32;
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/4;							

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 1]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%128)/16;	

	}else if(actual_neighbour == 2){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/8;
		longitud = rails[(point.y*basic_width + point.x)*6 + 2]%8;		

	}else if(actual_neighbour == 4){

		movement = rails[(point.y*basic_width + point.x)*6 + 3]/32;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/4;	

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/16;

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;
		longitud = rails[(point.y*basic_width + point.x)*6 + 5]%8;	

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_9(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = rails[(point.y*basic_width + point.x)*6]/32;
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/4;							

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 1]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%128)/16;	

	}else if(actual_neighbour == 2){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/8;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%8)/2;		

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 3]/64);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%64)/16;

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%2)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/16;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/4;

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 5]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%128)/32;

	}else if(actual_neighbour == 8){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%32)/4;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%4);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_10(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/8;

	}else if(actual_neighbour == 1){

		movement = (rails[(point.y*basic_width + point.x)*6]%8);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]/64);	

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%64)/8;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%8)/2;

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/16;

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%2)*2 + (rails[(point.y*basic_width + point.x)*6 + 3]/128);

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%128)/16;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/4;

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/32;

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%32)/4;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%4);

	}else if(actual_neighbour == 8){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 5])/32;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%32)/16;

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%2);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_11(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/8;

	}else if(actual_neighbour == 1){

		movement = (rails[(point.y*basic_width + point.x)*6]%8);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]/64);	

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%64)/8;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%8)/2;

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/16;

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%2);

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/16;

	}else if(actual_neighbour == 6){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%2);

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%32)/16;

	}else if(actual_neighbour == 8){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%2);

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%32)/16;

	}else if(actual_neighbour == 10){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%2);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_12(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/16;

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6]%2);

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%32)/16;				

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%2);

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%32)/16;

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%2);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/16;

	}else if(actual_neighbour == 7){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%2);

	}else if(actual_neighbour == 8){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%32)/16;

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%2);

	}else if(actual_neighbour == 10){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%32)/16;

	}else if(actual_neighbour == 11){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 5]%2);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_13(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud=0;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/16;

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6]%2);

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%32)/16;				

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%2);

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%32)/16;

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%2);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/16;

	}else if(actual_neighbour == 7){
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 3]%2);

	}else if(actual_neighbour == 8){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 4]%32)/16;

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;		

	}else if(actual_neighbour == 10){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 11){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;

	}else if(actual_neighbour == 12){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%8);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_14(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud=0;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/16;

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6]%2);

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%32)/16;				

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%2);

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%32)/16;

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6 + 2]%2);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);		

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/4;

	}else if(actual_neighbour == 8){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/16;

	}else if(actual_neighbour == 10){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;		

	}else if(actual_neighbour == 11){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 12){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;

	}else if(actual_neighbour == 13){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%8);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_15(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;
	unsigned char longitud=0;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6]%32)/16;

	}else if(actual_neighbour == 1){
	
		movement = (rails[(point.y*basic_width + point.x)*6]%16)/2;
		longitud = (rails[(point.y*basic_width + point.x)*6]%2);

	}else if(actual_neighbour == 2){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]/32);
		longitud = (rails[(point.y*basic_width + point.x)*6 + 1]%32)/16;					

	}else if(actual_neighbour == 3){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;	

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/8;

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%8);

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);		

	}else if(actual_neighbour == 8){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/4;

	}else if(actual_neighbour == 9){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);

	}else if(actual_neighbour == 10){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/16;

	}else if(actual_neighbour == 11){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;		

	}else if(actual_neighbour == 12){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 13){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;

	}else if(actual_neighbour == 14){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%8);

	}

	for(int j = 0 ; j <= longitud; j++){
		nuevo_punto = GetNextLoc(nuevo_punto, movement);
		steps.push_back(nuevo_punto);			
	}
}
void ReadRails_16(int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){

	unsigned char movement;	
	xyLoc nuevo_punto = point;
	
	if(actual_neighbour == 0){

		movement = (rails[(point.y*basic_width + point.x)*6]/32);		

	}else if(actual_neighbour == 1){

		movement = (rails[(point.y*basic_width + point.x)*6]%32)/4;

	}else if(actual_neighbour == 2){	

		movement = (rails[(point.y*basic_width + point.x)*6]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 1]/128);

	}else if(actual_neighbour == 3){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%128)/16;

	}else if(actual_neighbour == 4){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%16)/2;		

	}else if(actual_neighbour == 5){

		movement = (rails[(point.y*basic_width + point.x)*6 + 1]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 2]/64);

	}else if(actual_neighbour == 6){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%64)/8;

	}else if(actual_neighbour == 7){

		movement = (rails[(point.y*basic_width + point.x)*6 + 2]%8);

	}else if(actual_neighbour == 8){	
		
		movement = (rails[(point.y*basic_width + point.x)*6 + 3]/32);		

	}else if(actual_neighbour == 9){

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%32)/4;

	}else if(actual_neighbour == 10){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 3]%4)*2 + (rails[(point.y*basic_width + point.x)*6 + 4]/128);

	}else if(actual_neighbour == 11){	

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%128)/16;

	}else if(actual_neighbour == 12){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%16)/2;		

	}else if(actual_neighbour == 13){

		movement = (rails[(point.y*basic_width + point.x)*6 + 4]%2)*4 + (rails[(point.y*basic_width + point.x)*6 + 5]/64);

	}else if(actual_neighbour == 14){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%64)/8;

	}else if(actual_neighbour == 15){

		movement = (rails[(point.y*basic_width + point.x)*6 + 5]%8);

	}
	
	nuevo_punto = GetNextLoc(nuevo_punto, movement);
	steps.push_back(nuevo_punto);			
}
void ReadRails(int total_neighbours, int actual_neighbour, xyLoc point, std::vector<xyLoc> & steps){
	
	switch(total_neighbours){
		case 1:
			ReadRails_1(actual_neighbour, point, steps);
			break;
		case 2:
			ReadRails_2(actual_neighbour, point, steps);
			break;
		case 3:
			ReadRails_3(actual_neighbour, point, steps);
			break;
		case 4:
			ReadRails_4(actual_neighbour, point, steps);
			break;
		case 5:
			ReadRails_5(actual_neighbour, point, steps);
			break;
		case 6:
			ReadRails_6(actual_neighbour, point, steps);
			break;
		case 7:
			ReadRails_7(actual_neighbour, point, steps);
			break;
		case 8:
			ReadRails_8(actual_neighbour, point, steps);
			break;
		case 9:
			ReadRails_9(actual_neighbour, point, steps);
			break;
		case 10:
			ReadRails_10(actual_neighbour, point, steps);
			break;
		case 11:
			ReadRails_11(actual_neighbour, point, steps);
			break;
		case 12:
			ReadRails_12(actual_neighbour, point, steps);
			break;
		case 13:
			ReadRails_13(actual_neighbour, point, steps);
			break;
		case 14:
			ReadRails_14(actual_neighbour, point, steps);
			break;	
		case 15:
			ReadRails_15(actual_neighbour, point, steps);
			break;
		default:
			ReadRails_16(actual_neighbour, point, steps);
			break;
	}	
}

int SacarGrupoInsignia(std::vector<int>* grupoauxiliar_grupo, int id_grupo){

	int insignia = (*grupoauxiliar_grupo)[id_grupo];
	while((*grupoauxiliar_grupo)[insignia] != insignia){
		insignia = (*grupoauxiliar_grupo)[insignia];
	}
	
	if(insignia != (*grupoauxiliar_grupo)[id_grupo]){
		(*grupoauxiliar_grupo)[id_grupo] = insignia;
	}

	return insignia;
}

void SubPreprocessMapGroups(std::vector< std::vector<int> >* mini_mapag, int contador, int mini_width, int mini_height, int tope, int * mini_grupos_conseguidos){

	std::vector< int > tamanyos(contador+1, 1);		// tamaño de cada grupo
	//tamanyos.resize(contador+1, 1);

	std::vector< std::list<grupo> > q;	// array de listas de grupos, las listas están ordenadas por tamaño del borde
	q.resize(mini_width*mini_height*2);

	grupo A;	// siguiente grupo a analizar
	grupo B;	// pareja del grupo analizado

	int min = 4;		// frontera inicial

	std::vector< std::list<xyLoc> > puntos_grupo;	// array de listas de puntos limitrofes de los grupos
	puntos_grupo.resize(contador+1);

	std::vector< std::list<grupo>::iterator > punteros_grupos;	// iterator de cada grupo dentro de q
	punteros_grupos.resize(contador+1);

	// la id ficticia del grupo
	std::vector<int> * grupoauxiliar_grupo;
	grupoauxiliar_grupo = new std::vector<int>;
	(*grupoauxiliar_grupo).resize(contador+1);
	for(int i = 0; i<contador+1; i++){
		(*grupoauxiliar_grupo)[i] = i;
	}

	for(int i = 0; i<mini_width; i++){
		for(int j = 0; j<mini_height; j++){
			if((*mini_mapag)[i][j] != 0){
				grupo nue_g;
				nue_g.id = (*mini_mapag)[i][j];
				nue_g.borde = 4;
				nue_g.tamanyo = 1;

				//tamanyos[nue_g.id] = 1;

				xyLoc punto;
				punto.x = i;
				punto.y = j;
				puntos_grupo[nue_g.id].push_back(punto);				
				
				q[nue_g.borde].push_back(nue_g);

				punteros_grupos[nue_g.id] = (q[4].end());
				advance(punteros_grupos[nue_g.id], -1);			
			}
		}
	}

	std::vector<int> vecinos;
	vecinos.assign(contador+1, 0);

	int contador_real = contador;
	std::list<xyLoc>::iterator it_d;
	while(contador > tope && contador_real>1){
		A = q[min].front();
		q[min].pop_front();

		vecinos.assign(vecinos.size(), 0);

		int max_fronteras = 0;
		int max_id = 0;
		int min_tamanyo = mini_width*mini_height;

		bool eliminar = false;
		
		it_d = puntos_grupo[A.id].begin();
			
		int grupo_auxiliar_vecino = 0;	// donde vamos guardando el id real del vecino

		while(it_d != puntos_grupo[A.id].end()){
			
			eliminar = true;								

			if((*it_d).x>0){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, (*mini_mapag)[(*it_d).x-1][(*it_d).y]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
			if((*it_d).y>0){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, (*mini_mapag)[(*it_d).x][(*it_d).y-1]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;				
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
			if((*it_d).x<mini_width-1){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, (*mini_mapag)[(*it_d).x+1][(*it_d).y]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){		
					vecinos[grupo_auxiliar_vecino]++;					
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;					
				}
			}
			if((*it_d).y<mini_height-1){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, (*mini_mapag)[(*it_d).x][(*it_d).y+1]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;			
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
		
			if(eliminar == true){
				it_d = puntos_grupo[A.id].erase(it_d);						
			}else{
				it_d++;
			}
		}

		if(max_id == 0){					
			contador_real--;

			while(q[min].size() == 0){
				min+=2;
			}

		}else{

			B.id = max_id;

			B.borde = (*(punteros_grupos[max_id])).borde;
			q[B.borde].erase(punteros_grupos[max_id]);

			int old_g_id;
			int new_g_id;
			if(A.id < B.id){
				old_g_id = B.id;
				new_g_id = A.id;
			}else{
				old_g_id = A.id;
				new_g_id = B.id;
			}			

			(*grupoauxiliar_grupo)[old_g_id] = (*grupoauxiliar_grupo)[new_g_id];

			grupo C;
			C.id = new_g_id;
			C.borde = A.borde + B.borde - max_fronteras - max_fronteras;
			tamanyos[C.id] = tamanyos[A.id] + tamanyos[B.id];
			
			std::list<xyLoc>::iterator it_d2 = puntos_grupo[old_g_id].begin();
			while(it_d2 != puntos_grupo[old_g_id].end()){
				puntos_grupo[new_g_id].push_back((*it_d2));
				it_d2++;
			}
			puntos_grupo[old_g_id].clear();

			q[C.borde].push_back(C);
			punteros_grupos[C.id] = q[C.borde].end();
			advance(punteros_grupos[C.id], -1);	

			
			while(q[min].size() == 0){
				min+=2;
			}			
			contador--;
			contador_real--;
		}
	}
	
	tope = contador;

	// ***

	int aux_libre = 0;

	std::vector<int> grupos_libres;	
	for(int i = 2; i<(int)(*grupoauxiliar_grupo).size(); i++){
		int j = (*grupoauxiliar_grupo)[i];
		if(j != i){
			grupos_libres.push_back(i);
			(*grupoauxiliar_grupo)[i] = (*grupoauxiliar_grupo)[j];
		}else if((int)grupos_libres.size()> aux_libre){
			grupos_libres.push_back(i);
			(*grupoauxiliar_grupo)[i] = grupos_libres[aux_libre++];
		}
	}

	for(int i = 0; i<mini_width; i++){
		for(int j = 0; j<mini_height; j++){
			(*mini_mapag)[i][j] = (*grupoauxiliar_grupo)[(*mini_mapag)[i][j]];					
		}
	}	

	(*mini_grupos_conseguidos) = tope;
	delete grupoauxiliar_grupo;
}

void HeadSubPreprocessMapGroupsBase(std::deque< thread_data >* hilo_head_data){
	
	std::deque<thread_data >::iterator itr_m = (*hilo_head_data).begin();
	while(itr_m != (*hilo_head_data).end()){			

		std::thread tmini(SubPreprocessMapGroups, (*itr_m).mini_mapag, (*itr_m).contador, (*itr_m).width, (*itr_m).height, (*itr_m).tope, (*itr_m).mini_grupos_conseguidos);
		tmini.join();
		
		advance(itr_m,1);
	}
}

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename){
	basic_width = width;
	basic_height = height;

	int count_cells = 0;
	for(int i = 0; i<width*height; i++){
		if(bits[i] == true){
			count_cells++;
		}
	}
	if(TOPE > count_cells){
		TOPE = count_cells;
	}

	std::vector< std::list<xyLoc> > puntos_frontera;	// array de listas de puntos limitrofes de los grupos
	std::vector< xyLoc > puntos_conexion;
	std::vector< std::list<xyLoc> > puntos_grupo;	// array de listas de puntos limitrofes de los grupos

	Preprocess_Part1(bits, width, height, puntos_frontera, puntos_grupo, puntos_conexion);	
	Preprocess_Part2(width, height, puntos_frontera, puntos_grupo, puntos_conexion);
	Preprocess_Part3(width, height, filename);
}

void Preprocess_Part1(std::vector<bool> &bits, int width, int height, std::vector< std::list<xyLoc> > &puntos_frontera, std::vector< std::list<xyLoc> > &puntos_grupo, std::vector< xyLoc > &puntos_conexion){	

	std::vector< std::vector<int> > mapag;
	mapag.resize(width);				// asignacion de punto-grupo
	for(int i = 0; i<width; i++){
		mapag[i].resize(height, 0);		// se inicializan como grupo: obstáculo
	}

	int contador = 0;

	for(int i=0; i<(int)bits.size(); i++){
		if(bits[i]){
			contador++;
			mapag[i%width][i/width] = contador;
		}else{
			mapag[i%width][i/width] = 0;
		}
	}

	std::vector< std::vector< std::vector< std::vector<int> >* > > mini_mapgs;
	std::vector< std::vector<int> > mini_contadores;
	std::vector< std::vector< int * > > mini_grupos_conseguidos;

	int tope_i = width/SEPARADOR;
	if(width%SEPARADOR > 0){
		tope_i++;
	}
	int tope_j = height/SEPARADOR;
	if(height%SEPARADOR > 0){
		tope_j++;
	}

	mini_mapgs.resize(tope_i);
	mini_contadores.resize(tope_i);
	mini_grupos_conseguidos.resize(tope_i);
	for(int i = 0; i<tope_i; i++){
		mini_mapgs[i].resize(tope_j);
		mini_contadores[i].resize(tope_j);
		mini_grupos_conseguidos[i].resize(tope_j);
		for(int j = 0; j<tope_j; j++){
			mini_grupos_conseguidos[i][j] = new int;
			(*mini_grupos_conseguidos[i][j]) = 0;
		}
	}

	for(int i = 0; i<tope_i; i++){
		for(int j=0; j<tope_j; j++){
			int mini_width = SEPARADOR;
			int mini_height = SEPARADOR;
			if(i == tope_i-1 && width%SEPARADOR > 0){
				mini_width = width%SEPARADOR;
			}
			if(j == tope_j-1 && height%SEPARADOR > 0){
				mini_height = height%SEPARADOR;
			}
			
			mini_mapgs[i][j] = new std::vector< std::vector<int> >;
			(*mini_mapgs[i][j]).resize(mini_width);
			for(int k = 0; k<mini_width; k++){
				(*mini_mapgs[i][j])[k].resize(mini_height);
			}
			mini_contadores[i][j] = 0;
			for(int k = 0; k<mini_width; k++){
				for(int l = 0; l<mini_height; l++){
					
					if(mapag[i*SEPARADOR + k][j*SEPARADOR + l] > 0){
						mini_contadores[i][j]++;
						(*mini_mapgs[i][j])[k][l] = mini_contadores[i][j];
					}else{
						(*mini_mapgs[i][j])[k][l] = 0;
					}
				}
			}
		}
	}

	std::vector< std::deque < thread_data >* > head_threads_data;
	head_threads_data.resize(NUM_THREADS);
	for(int i = 0; i<NUM_THREADS; i++){
		head_threads_data[i] = new std::deque<thread_data >;
	}

	std::vector< std::thread* > head_threads;
	head_threads.resize(NUM_THREADS);

	int current_thread = 0;

	for(int i = 0; i<tope_i; i++){
		for(int j = 0; j<tope_j; j++){
			int aux_width = (int)(*mini_mapgs[i][j]).size();
			int aux_height = (int)(*mini_mapgs[i][j])[0].size();
			
			if(mini_contadores[i][j] > GRUPOS_HILO){
				thread_data head_thread_data_aux;

				head_thread_data_aux.mini_mapag = mini_mapgs[i][j];
				head_thread_data_aux.contador = mini_contadores[i][j];
				head_thread_data_aux.width = aux_width;
				head_thread_data_aux.height = aux_height;
				head_thread_data_aux.tope = GRUPOS_HILO;
				head_thread_data_aux.mini_grupos_conseguidos = mini_grupos_conseguidos[i][j];
				(*head_threads_data[current_thread%NUM_THREADS]).push_back(head_thread_data_aux);
				current_thread++;
			}else{
				(*mini_grupos_conseguidos[i][j]) = mini_contadores[i][j];
			}
		}
	}

	for(int i = 0; i < NUM_THREADS; i++){	
		head_threads[i] = new std::thread(HeadSubPreprocessMapGroupsBase, head_threads_data[i]);
	}
	for(int i = 0; i < NUM_THREADS; i++){
		(*head_threads[i]).join();
	}
	
	// primera fase
	
	std::vector< std::list<grupo> > q;	// array de listas de grupos, las listas están ordenadas por tamaño del borde
	q.resize(width*height*2);

	grupo A;	// siguiente grupo a analizar
	grupo B;	// pareja del grupo analizado

	contador = 0;	// grupos actuales
	int tope = TOPE;		// top de grupos
	int min = width*height*2;		// frontera inicial

	for(int i = 0; i<tope_i; i++){
		int aux_width = SEPARADOR;
		if(i == tope_i-1 && width%SEPARADOR > 0){
			aux_width = width%SEPARADOR;
		}
		for(int j=0; j<tope_j; j++){
			int aux_height = SEPARADOR;
			if(j == tope_j-1 && height%SEPARADOR > 0){
				aux_height = height%SEPARADOR;
			}
			for(int k = 0; k<aux_width; k++){
				for(int l = 0; l<aux_height; l++){
					if((*mini_mapgs[i][j])[k][l] == 0){
						mapag[i*SEPARADOR + k][j*SEPARADOR + l] = 0;
					}else{
						mapag[i*SEPARADOR + k][j*SEPARADOR + l] = (*mini_mapgs[i][j])[k][l] + contador;
					}
				}
			}
			contador += (*mini_grupos_conseguidos[i][j]);
		}
	}

	if(contador <= tope){
		tope = contador;
	}

	std::vector< int > tamanyos;		// tamaño de cada grupo
	tamanyos.resize(contador+1, 0); 

	
	puntos_grupo.resize(contador+1);

	std::vector< std::list<grupo>::iterator > punteros_grupos;	// iterator de cada grupo dentro de q
	punteros_grupos.resize(contador+1);

	std::vector<int> * grupoauxiliar_grupo;
	grupoauxiliar_grupo = new std::vector<int>;
	(*grupoauxiliar_grupo).resize(contador+1);	
	for(int i = 0; i<contador+1; i++){
		(*grupoauxiliar_grupo)[i] = i;
	}

	std::vector< grupo > grupos_iniciales_multipunto;	// array de grupos multipuntos iniciales
	grupos_iniciales_multipunto.resize(contador+1);

	for(int i = 0; i<width; i++){
		for(int j = 0; j<height; j++){
			if(mapag[i][j] != 0){
				if(tamanyos[mapag[i][j]] == 0){
					grupo nue_g;
					nue_g.id = mapag[i][j];
					nue_g.borde = 0;

					if(i>0){
						if(mapag[i-1][j] != mapag[i][j]){
							nue_g.borde++;
						}
					}
					if(i == 0){
						nue_g.borde++;
					}
					if(j>0){
						if(mapag[i][j-1] != mapag[i][j]){
							nue_g.borde++;
						}
					}
					if(j == 0){
						nue_g.borde++;
					}
					if(i<width-1){
						if(mapag[i+1][j] != mapag[i][j]){
							nue_g.borde++;
						}
					}
					if(i == width-1){
						nue_g.borde++;
					}
					if(j<height-1){
						if(mapag[i][j+1] != mapag[i][j]){
							nue_g.borde++;
						}
					}
					if(j == height-1){
						nue_g.borde++;
					}		
					grupos_iniciales_multipunto[mapag[i][j]] = nue_g;
					tamanyos[nue_g.id] = 1;
					
					if(nue_g.borde > 0){
						xyLoc punto;
						punto.x = i;
						punto.y = j;				
						puntos_grupo[nue_g.id].push_back(punto);		
					}							
				}else{
					int aux_borde = 0;
					
					if(i>0){
						if(mapag[i-1][j] != mapag[i][j]){
							aux_borde++;
						}
					}
					if(i == 0){
						aux_borde++;
					}
					if(j>0){
						if(mapag[i][j-1] != mapag[i][j]){
							aux_borde++;
						}
					}
					if(j == 0){
						aux_borde++;
					}
					if(i<width-1){
						if(mapag[i+1][j] != mapag[i][j]){
							aux_borde++;
						}
					}
					if(i == width-1){
						aux_borde++;
					}
					if(j<height-1){
						if(mapag[i][j+1] != mapag[i][j]){
							aux_borde++;
						}
					}	
					if(j == height-1){
						aux_borde++;
					}									

					tamanyos[mapag[i][j]]++;

					if(aux_borde > 0){
						xyLoc punto;
						punto.x = i;
						punto.y = j;
						puntos_grupo[mapag[i][j]].push_back(punto);
						grupos_iniciales_multipunto[mapag[i][j]].borde += aux_borde;
					}
				}		
			}
		}
	}

	for(int i = 1; i<=contador; i++){

		grupos_iniciales_multipunto[i].tamanyo = tamanyos[i];
		int borde_aux = grupos_iniciales_multipunto[i].borde;
		q[borde_aux].push_back(grupos_iniciales_multipunto[i]);

		punteros_grupos[i] = (q[borde_aux].end());
		advance(punteros_grupos[i], -1);

		if(min > borde_aux){
			min = borde_aux;		
		}
	}

	std::vector<int> vecinos;
	vecinos.assign(contador+1, 0);

	while(contador > tope){

		A = q[min].front();
		q[min].pop_front();

		vecinos.assign(vecinos.size(), 0);

		int max_fronteras = 0;
		int max_id = 0;
		int min_tamanyo = width*height;

		bool eliminar = false;
		
		std::list<xyLoc>::iterator it_d = puntos_grupo[A.id].begin();
			
		int grupo_auxiliar_vecino = 0;	// donde vamos guardando el id real del vecino

		for( ; it_d != puntos_grupo[A.id].end(); ){
			
			eliminar = true;								

			if((*it_d).x>0){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, mapag[(*it_d).x-1][(*it_d).y]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
			if((*it_d).y>0){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, mapag[(*it_d).x][(*it_d).y-1]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;				
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
			if((*it_d).x<width-1){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, mapag[(*it_d).x+1][(*it_d).y]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){		
					vecinos[grupo_auxiliar_vecino]++;					
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;					
				}
			}
			if((*it_d).y<height-1){
				grupo_auxiliar_vecino = SacarGrupoInsignia(grupoauxiliar_grupo, mapag[(*it_d).x][(*it_d).y+1]);
				if(grupo_auxiliar_vecino != A.id && grupo_auxiliar_vecino != 0){
					vecinos[grupo_auxiliar_vecino]++;			
					if(vecinos[grupo_auxiliar_vecino] > max_fronteras || (vecinos[grupo_auxiliar_vecino] == max_fronteras && tamanyos[grupo_auxiliar_vecino]<min_tamanyo)){
						max_fronteras = vecinos[grupo_auxiliar_vecino];
						max_id = grupo_auxiliar_vecino;
						min_tamanyo = tamanyos[grupo_auxiliar_vecino];
					}
					eliminar = false;
				}
			}
		
			if(eliminar == true){
				it_d = puntos_grupo[A.id].erase(it_d);
			}else{
				it_d++;
			}
		}

		if(max_id == 0){					
			while(q[min].size() == 0){
				min+=2;
			}
		}else{

			B.id = max_id;

			B.borde = (*(punteros_grupos[max_id])).borde;
			q[B.borde].erase(punteros_grupos[max_id]);

			int old_g_id;
			int new_g_id;
			if(A.id < B.id){
				old_g_id = B.id;
				new_g_id = A.id;
			}else{
				old_g_id = A.id;
				new_g_id = B.id;
			}			

			(*grupoauxiliar_grupo)[old_g_id] = (*grupoauxiliar_grupo)[new_g_id];

			grupo C;
			C.id = new_g_id;
			C.borde = A.borde + B.borde - max_fronteras - max_fronteras;
			tamanyos[C.id] = tamanyos[A.id] + tamanyos[B.id];
			
			std::list<xyLoc>::iterator it_d2 = puntos_grupo[old_g_id].begin();
			while(it_d2 != puntos_grupo[old_g_id].end()){
				puntos_grupo[new_g_id].push_back((*it_d2));
				it_d2++;
			}

			puntos_grupo[old_g_id].clear();

			q[C.borde].push_back(C);
			punteros_grupos[C.id] = q[C.borde].end();
			advance(punteros_grupos[C.id], -1);	

			
			while(q[min].size() == 0){
				min+=2;
			}			
			contador--;
		}
	}

	/////

	int aux_libre = 0;

	std::vector<int> grupos_libres;
	for(int i = 2; i<(int)(*grupoauxiliar_grupo).size(); i++){
		int j = (*grupoauxiliar_grupo)[i];
		if(j != i){
			grupos_libres.push_back(i);
			(*grupoauxiliar_grupo)[i] = (*grupoauxiliar_grupo)[j];
		}else if((int)grupos_libres.size()> aux_libre){
			grupos_libres.push_back(i);
			(*grupoauxiliar_grupo)[i] = grupos_libres[aux_libre++];
		}
	}

	for(int i = 0; i<width; i++){
		for(int j = 0; j<height; j++){
			(mapag)[i][j] = (*grupoauxiliar_grupo)[(mapag)[i][j]];
		}
	}

	// se han recolocado los id_grupo para cada punto del mapa

	for(int i = 0; i<(int)puntos_grupo.size(); i++){
		puntos_grupo[i].clear();
	}
	puntos_grupo.resize(tope+1);						// array de puntos del grupo

	
	puntos_frontera.resize(tope+1);

	std::vector< std::list<xyLoc> > puntos_borde;	// array de listas de puntos limitrofes de los grupos
	puntos_borde.resize(tope+1);

	map.resize(width*height, 0);

	int punto_actual = 0;
	bool anyadir_punto_frontera;
	bool anyadir_punto_borde;
	for(int i = 0; i<width; i++){
		for(int j = 0; j<height; j++){
			punto_actual = (mapag)[i][j];
			if(punto_actual != 0){

				xyLoc punto_aux;
				punto_aux.x = i;
				punto_aux.y = j;
				puntos_grupo[punto_actual].push_back(punto_aux);
				map[j*width+i] = punto_actual;
				anyadir_punto_frontera = false;
				anyadir_punto_borde = false;
				if(i > 0){
					if((mapag)[i-1][j]!= 0 && (mapag)[i-1][j] != punto_actual){
						anyadir_punto_frontera = true;
					}
					if((mapag)[i-1][j] != punto_actual){
						anyadir_punto_borde = true;
					}
				}
				if(j > 0){
					if((mapag)[i][j-1]!= 0 && (mapag)[i][j-1] != punto_actual){
						anyadir_punto_frontera = true;
					}
					if((mapag)[i][j-1] != punto_actual){
						anyadir_punto_borde = true;
					}
				}
				if(i < width-1){
					if((mapag)[i+1][j]!= 0 && (mapag)[i+1][j] != punto_actual){
						anyadir_punto_frontera = true;
					}
					if((mapag)[i+1][j] != punto_actual){
						anyadir_punto_borde = true;
					}					
				}
				if(j < height-1){
					if((mapag)[i][j+1]!= 0 && (mapag)[i][j+1] != punto_actual){
						anyadir_punto_frontera = true;
					}
					if((mapag)[i][j+1] != punto_actual){
						anyadir_punto_borde = true;
					}					
				}
				if(i > 0 && j > 0){
					if((mapag)[i-1][j-1] != punto_actual){
						anyadir_punto_borde = true;
					}
				}
				if(i < width-1 && j > 0){
					if((mapag)[i+1][j-1] != punto_actual){
						anyadir_punto_borde = true;
					}
				}
				if(i > 0 && j < height-1){
					if((mapag)[i-1][j+1] != punto_actual){
						anyadir_punto_borde = true;
					}
				}		
				if(i < width-1 && j < height-1){
					if((mapag)[i+1][j+1] != punto_actual){
						anyadir_punto_borde = true;
					}
				}			
				if(i == 0 || j == 0 || i == width-1 || j == height-1){
					anyadir_punto_borde = true;
				}
				if(anyadir_punto_borde == true){								
					puntos_borde[punto_actual].push_back(punto_aux);										
				}
				if(anyadir_punto_frontera == true){				
					puntos_frontera[punto_actual].push_back(punto_aux);					
				}
			}
		}
	}

	// para cada grupo tenemos sus puntos totales, de frontera y de borde

	////
	////
	////
	////
	////
	////
	
	std::list<xyLoc>::iterator it_d3;

	std::vector< std::vector<bool> > es_vecino;	// por cada grupo un un array de bool que dice si otro grupo es vecino suyo
	es_vecino.resize(tope+1);
	for(int i = 0; i<tope+1; i++){
		es_vecino[i].resize(tope+1, false);
	}	
		
	int num_vecinos = 0;
	neighbours_number.resize(tope+1, (unsigned char)0);

	for(int i = 1; i<=tope; i++){
		it_d3 = puntos_frontera[i].begin();
		num_vecinos = 0;

		while(it_d3 != puntos_frontera[i].end()){
			if((*it_d3).x>0){
				if(mapag[(*it_d3).x-1][(*it_d3).y] != 0 && mapag[(*it_d3).x-1][(*it_d3).y] != i){
					if(es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y]] = true;
					}					
				}					
			}
			if((*it_d3).y>0){
				if(mapag[(*it_d3).x][(*it_d3).y-1] != 0 && mapag[(*it_d3).x][(*it_d3).y-1] != i){
					if(es_vecino[i][mapag[(*it_d3).x][(*it_d3).y-1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x][(*it_d3).y-1]] = true;		
					}					
				}
			}
			if((*it_d3).x<width-1){
				if(mapag[(*it_d3).x+1][(*it_d3).y] !=0 && mapag[(*it_d3).x+1][(*it_d3).y] !=i){
					if(es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y]] = true;
					}
				}
			}
			if((*it_d3).y<height-1){
				if(mapag[(*it_d3).x][(*it_d3).y+1] != 0 && mapag[(*it_d3).x][(*it_d3).y+1] != i){
					if(es_vecino[i][mapag[(*it_d3).x][(*it_d3).y+1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x][(*it_d3).y+1]] = true;
					}
				}
			}
			if((*it_d3).x>0 && (*it_d3).y>0 && mapag[(*it_d3).x-1][(*it_d3).y] != 0 && mapag[(*it_d3).x][(*it_d3).y-1] != 0){
				if(mapag[(*it_d3).x-1][(*it_d3).y-1] != 0 && mapag[(*it_d3).x-1][(*it_d3).y-1] != i){
					if(es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y-1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y-1]] = true;					
					}						
				}					
			}
			if((*it_d3).x>0 && (*it_d3).y<height-1 && mapag[(*it_d3).x-1][(*it_d3).y] != 0 && mapag[(*it_d3).x][(*it_d3).y+1] != 0){
				if(mapag[(*it_d3).x-1][(*it_d3).y+1] != 0 && mapag[(*it_d3).x-1][(*it_d3).y+1] != i){
					if(es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y+1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x-1][(*it_d3).y+1]] = true;	
					}				
				}
			}
			if((*it_d3).x<width-1 && (*it_d3).y>0 && mapag[(*it_d3).x+1][(*it_d3).y] != 0 && mapag[(*it_d3).x][(*it_d3).y-1] != 0){
				if(mapag[(*it_d3).x+1][(*it_d3).y-1] !=0 && mapag[(*it_d3).x+1][(*it_d3).y-1] !=i){
					if(es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y-1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y-1]] = true;
					}

				}
			}
			if((*it_d3).x<width-1 && (*it_d3).y<height-1 && mapag[(*it_d3).x+1][(*it_d3).y] != 0 && mapag[(*it_d3).x][(*it_d3).y+1] != 0){
				if(mapag[(*it_d3).x+1][(*it_d3).y+1] != 0 && mapag[(*it_d3).x+1][(*it_d3).y+1] != i){
					if(es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y+1]] == false){
						num_vecinos++;
						es_vecino[i][mapag[(*it_d3).x+1][(*it_d3).y+1]] = true;
					}
				}
			}								
			it_d3++;
		}	

		neighbours_number[i] = (unsigned char) num_vecinos;

	}

	neighbours_groupID.resize((tope+1)*MAX_VECINOS, 0);

	int neighbor_aux_index;

	for(int i = 1; i<=tope; i++){
		neighbor_aux_index = 0;
		for(int j = 1; j<=tope; j++){
			if(i!=j){
				if(es_vecino[i][j] == true){
					neighbours_groupID[i*MAX_VECINOS+neighbor_aux_index] = j;
					neighbor_aux_index++;
				}
			}
		}
	}

	///  PUNTOS CONEXION

	xyLoc xyNegativo;
	xyNegativo.x = -1;
	xyNegativo.y = -1;
	puntos_conexion.resize((tope+1)*MAX_VECINOS);
	for(unsigned int i = 0; i< puntos_conexion.size(); i++){
		puntos_conexion[i].x = -1;
		puntos_conexion[i].y = -1;
	}

	int grupo_frontera_vecino;

	for(int i = 1; i<=tope; i++){		
		for(int j = 0; j<neighbours_number[i] ; j++){			
			if(puntos_conexion[i*MAX_VECINOS+j].x == -1){
				grupo_frontera_vecino = neighbours_groupID[i*MAX_VECINOS+j];

				double coor_x1 = 0.0;
				double coor_y1 = 0.0;
				xyNegativo.x = 0;
				xyNegativo.y = 0;

				int contador_coord = 0;

				bool punto_vecino;

				std::list<xyLoc>::iterator it_l11 = puntos_frontera[i].begin();

				while(it_l11 != puntos_frontera[i].end()){				
					GetSuccessors((*it_l11), succ);		
					punto_vecino = false;		
					for (unsigned int k = 0; k < succ.size() && punto_vecino == false; k++){
						if(map[GetIndex(succ[k])] == grupo_frontera_vecino){
							contador_coord++;
							coor_x1 += (*it_l11).x;
							coor_y1 += (*it_l11).y;
							punto_vecino = true;
						}
					}
					it_l11++;
				}

				xyNegativo.x = (int)(xyNegativo.x/contador_coord);
				xyNegativo.y = (int)(xyNegativo.y/contador_coord);

				it_l11 = puntos_frontera[i].begin();

				int distancia_coord = 8000;

				xyLoc punto_elegido;

				while(it_l11 != puntos_frontera[i].end()){				
					GetSuccessors((*it_l11), succ);
					punto_vecino = false;		
					for (unsigned int k = 0; k < succ.size() && punto_vecino == false; k++){
						if(map[GetIndex(succ[k])] == grupo_frontera_vecino){

							if(ManhattanDistance((*it_l11), xyNegativo) < distancia_coord){
								punto_elegido = (*it_l11);
								distancia_coord = ManhattanDistance((*it_l11), xyNegativo);
							}

							punto_vecino = true;
						}
					}
					it_l11++;
				}

				puntos_conexion[i*MAX_VECINOS+j] = punto_elegido;				

				GetSuccessors(punto_elegido, succ);
				punto_vecino = false;		
				for (unsigned int k = 0; k < succ.size() && punto_vecino == false; k++){
					if(map[GetIndex(succ[k])] == grupo_frontera_vecino){
						punto_elegido = succ[k];
						punto_vecino = true;
					}
				}

				int num_vecino_punto_elegido = -1;

				for(int k = 0; k<neighbours_number[grupo_frontera_vecino]; k++){
					if(neighbours_groupID[grupo_frontera_vecino*MAX_VECINOS+k] == i){
						num_vecino_punto_elegido = k;
					}
				}

				puntos_conexion[grupo_frontera_vecino*MAX_VECINOS+num_vecino_punto_elegido] = punto_elegido;				
			}
		}
	}	

	// segunda fase
	std::vector< std::vector<int> > grupobase_grupodestino_turnos;	// transito entre grupos (matriz de turnos para ir de un grupo a otro)
	
	group_distances.resize((tope+1)*(tope+1), 0);

	int asignaciones = tope*(tope-1);

	std::vector< std::vector<int> > spareBase;
	std::vector< std::vector<int> > spareAuxU;
	std::vector< std::vector<int> > spareMul;	

	spareBase.resize(tope+1);
	spareAuxU.resize(tope+1);
	spareMul.resize(tope+1);
	grupobase_grupodestino_turnos.resize(tope+1);

	for(int i = 1; i<tope+1; i++){
		
		spareBase[i].resize(tope+1,0);
		spareAuxU[i].resize(tope+1,0);
		spareMul[i].resize(tope+1,0);
		grupobase_grupodestino_turnos[i].resize(tope+1,0);
		
		for(int j = 1;j<tope+1;j++){
			if(es_vecino[i][j]){
				spareBase[i][j] = 1;
				spareAuxU[i][j] = 1;
				spareMul[i][j] = 1;
				grupobase_grupodestino_turnos[i][j] = 1;
				group_distances[i*(tope+1)+j] = 1;
				asignaciones--;
			}			
		}
	}

	int turno = 2;

	while(asignaciones > 0 && turno < tope){
		for(int i = 1; i<=tope;i++){
			for(int j = 1; j<=tope; j++){
				if(spareMul[i][j] == turno-1){
					for(int k = 1; k<=tope; k++){
						if(spareMul[j][k] == 1 && spareMul[i][k] == 0 && i!=k){							
							spareMul[i][k] = turno;
							group_distances[i*(tope+1)+k] = turno;
							asignaciones--;
						}
					}
				}
			}
		}
		turno++;
	}

	group_refloc.resize((tope+1)*2);

	/// centroides
	std::list<xyLoc>::iterator it_d4;
	int x_glob;
	int y_glob;
	int contador_puntos;
	for(int i = 1; i<=tope; i++){
		x_glob = 0;
		y_glob = 0;
		contador_puntos = 0;
		it_d4 = puntos_grupo[i].begin();
		while(it_d4 != puntos_grupo[i].end()){
			contador_puntos++;
			x_glob+=(*it_d4).x;
			y_glob+=(*it_d4).y;
			it_d4++;
		}		
		group_refloc[i*2]=(x_glob/contador_puntos);
		group_refloc[i*2+1]=(y_glob/contador_puntos);
	}	
}

void Preprocess_Part2(int width, int height, std::vector< std::list<xyLoc> > &puntos_frontera, std::vector< std::list<xyLoc> > &puntos_grupo, std::vector< xyLoc > &puntos_conexion){

	rails.resize(width*height*6, 0);

	std::vector< std::vector< bool > > railed_map;
	railed_map.resize(width);
	for(int i = 0; i<width; i++){
		railed_map[i].resize(height, false);		// se inicializan como grupo: obstáculo
	}

	std::vector< std::vector< std::deque<xyLoc> > > next_positions;
	next_positions.resize(width);
	for(int i = 0; i<width; i++){
		next_positions[i].resize(height);
	}

	std::deque< xyLoc > q_0;
	std::deque< xyLoc > q_1;
	std::list< xyLoc >::iterator it_d5;
	std::deque< xyLoc >::iterator it_d6;
	//bool vecino_encontrado;
	uint16_t vecino_id;
	xyLoc front_aux;

	for(int i = 1; i<=TOPE; i++){	
		
		for(int vecino_actual = 0; vecino_actual < neighbours_number[i] && vecino_actual < MAX_RAILS; vecino_actual++){
			
			vecino_id = neighbours_groupID[i*MAX_VECINOS+vecino_actual];

			xyLoc punto_frontera_rail = puntos_conexion[i*MAX_VECINOS+vecino_actual];
			xyLoc punto_vecino_frontera_rail;
				
			int num_vecino_punto_elegido = -1;

			for(int k = 0; k<neighbours_number[vecino_id]; k++){
				if(neighbours_groupID[vecino_id*MAX_VECINOS+k] == i){
					num_vecino_punto_elegido = k;
				}
			}

			punto_vecino_frontera_rail = puntos_conexion[vecino_id*MAX_VECINOS+num_vecino_punto_elegido];	
			
			//vecino_encontrado = false;
			//GetSuccessors(punto_frontera_rail, succ);
			
									
			//vecino_encontrado = true;
			railed_map[punto_frontera_rail.x][punto_frontera_rail.y] = true;						
			next_positions[punto_frontera_rail.x][punto_frontera_rail.y].push_front(punto_vecino_frontera_rail);
			WriteRails(neighbours_number[i], vecino_actual, punto_frontera_rail, next_positions[punto_frontera_rail.x][punto_frontera_rail.y]);
			q_0.push_back(punto_frontera_rail);
	
				
			while(q_0.empty() == false){
				it_d6 = q_0.begin();
				while(it_d6 != q_0.end()){
					GetSuccessors_cardinals((*it_d6), succ);
					for (unsigned int j = 0; j < succ.size(); j++){
						if(map[GetIndex(succ[j])] == i && railed_map[succ[j].x][succ[j].y] == false){
							railed_map[succ[j].x][succ[j].y] = true;
							next_positions[succ[j].x][succ[j].y] = next_positions[(*it_d6).x][(*it_d6).y];
							next_positions[succ[j].x][succ[j].y].push_front((*it_d6));
							WriteRails(neighbours_number[i], vecino_actual, succ[j], next_positions[succ[j].x][succ[j].y]);
							q_1.push_back(succ[j]);							
						}
					}
					it_d6++;
				}
				while(q_0.empty() == false){
					front_aux = q_0.front();
					q_0.pop_front();
					GetSuccessors_diagonals(front_aux, succ);
					for (unsigned int j = 0; j < succ.size(); j++){
						if(map[GetIndex(succ[j])] == i && railed_map[succ[j].x][succ[j].y] == false){
							railed_map[succ[j].x][succ[j].y] = true;
							next_positions[succ[j].x][succ[j].y] = next_positions[front_aux.x][front_aux.y];
							next_positions[succ[j].x][succ[j].y].push_front(front_aux);
							WriteRails(neighbours_number[i], vecino_actual, succ[j], next_positions[succ[j].x][succ[j].y]);
							q_1.push_back(succ[j]);							
						}
					}				
				}

				it_d6 = q_1.begin();
				while(it_d6 != q_1.end()){
					GetSuccessors_cardinals((*it_d6), succ);
					for (unsigned int j = 0; j < succ.size(); j++){
						if(map[GetIndex(succ[j])] == i && railed_map[succ[j].x][succ[j].y] == false){
							railed_map[succ[j].x][succ[j].y] = true;
							next_positions[succ[j].x][succ[j].y] = next_positions[(*it_d6).x][(*it_d6).y];
							next_positions[succ[j].x][succ[j].y].push_front((*it_d6));
							WriteRails(neighbours_number[i], vecino_actual, succ[j], next_positions[succ[j].x][succ[j].y]);
							q_0.push_back(succ[j]);							
						}
					}
					it_d6++;
				}
				while(q_1.empty() == false){
					front_aux = q_1.front();
					q_1.pop_front();
					GetSuccessors_diagonals(front_aux, succ);
					for (unsigned int j = 0; j < succ.size(); j++){
						if(map[GetIndex(succ[j])] == i && railed_map[succ[j].x][succ[j].y] == false){
							railed_map[succ[j].x][succ[j].y] = true;
							next_positions[succ[j].x][succ[j].y] = next_positions[front_aux.x][front_aux.y];
							next_positions[succ[j].x][succ[j].y].push_front(front_aux);
							WriteRails(neighbours_number[i], vecino_actual, succ[j], next_positions[succ[j].x][succ[j].y]);
							q_0.push_back(succ[j]);							
						}
					}				
				}
			}
			it_d5 = puntos_grupo[i].begin();
			while(it_d5 != puntos_grupo[i].end()){
				next_positions[(*it_d5).x][(*it_d5).y].clear();
				railed_map[(*it_d5).x][(*it_d5).y] = false;
				it_d5++;
			}
		}
	}	
}

void Preprocess_Part3(int width, int height, const char *filename){
	
	ofstream os_file;
	
	std::string file_s = filename;
	file_s += ".pre";

	os_file.open (file_s, ios::out | ios::binary | ios::trunc);

	os_file.write((const char*)&TOPE, sizeof(int));

	os_file.write((const char*)&map[0], width*height * sizeof(uint16_t));

	os_file.write((const char*)&neighbours_number[0], (TOPE+1) * sizeof(unsigned char));

	os_file.write((const char*)&neighbours_groupID[0], (TOPE+1)*MAX_VECINOS * sizeof(uint16_t));

	os_file.write((const char*)&group_distances[0], (TOPE+1)*(TOPE+1) * sizeof(uint16_t));

	os_file.write((const char*)&group_refloc[0], (TOPE+1)*2 * sizeof(uint16_t));

	os_file.write((const char*)&rails[0], width*height*6 * sizeof(unsigned char));

	os_file.close();
}

void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename){

	ifstream is_file;

	std::string file_s = filename;
	file_s += ".pre";

	map.resize(width*height, 0);
	neighbours_number.resize((TOPE+1), 0);
	neighbours_groupID.resize((TOPE+1)*MAX_VECINOS, 0);
	group_distances.resize((TOPE+1)*(TOPE+1), 0);
	group_refloc.resize((TOPE+1)*2, 0);
	rails.resize(width*height*6, 0);

	is_file.open(file_s, ios::binary); 

	is_file.read((char*)&TOPE, sizeof(int));

	is_file.read((char*)&map[0], width*height * sizeof(uint16_t));

	is_file.read((char*)&neighbours_number[0], (TOPE+1) * sizeof(unsigned char));

	is_file.read((char*)&neighbours_groupID[0], (TOPE+1)*MAX_VECINOS * sizeof(uint16_t));

	is_file.read((char*)&group_distances[0], (TOPE+1)*(TOPE+1) * sizeof(uint16_t));

	is_file.read((char*)&group_refloc[0], (TOPE+1)*2 * sizeof(uint16_t));

	is_file.read((char*)&rails[0], width*height*6 * sizeof(unsigned char));  

	is_file.close();

	visited.resize(width*height, 0);

	basic_width = width;
	basic_height = height;

	return (void *)13182;
}

int FindNextNeighbour(xyLoc origin, xyLoc goal){

	uint16_t id_grupo_actual = map[GetIndex(origin)];
	uint16_t id_grupo_meta = map[GetIndex(goal)];
	uint16_t vecino_siguiente;

	int distancia_centroide = 8000;
	int aux_distancia_centroide;
	int neighbour_number = -1;

	int saltos_grupos = group_distances[id_grupo_actual*(TOPE+1)+id_grupo_meta];

	if(saltos_grupos > 1){
		for(int i = 1; i<= TOPE; i++){
			if(group_distances[id_grupo_meta*(TOPE+1)+i] == (saltos_grupos - 1)){
				if(group_distances[id_grupo_actual*(TOPE+1)+i] == 1){
					xyLoc aux_ref;
					aux_ref.x = group_refloc[i*2];
					aux_ref.y = group_refloc[i*2+1];					
					aux_distancia_centroide = ManhattanDistance(origin,aux_ref);
					if(aux_distancia_centroide < distancia_centroide){
						distancia_centroide = aux_distancia_centroide;
						vecino_siguiente = i;
					}
				}
			}
		}
	}else{
		vecino_siguiente = id_grupo_meta;
	}

	for(int i = 0; i< neighbours_number[id_grupo_actual]; i++){		
		if(neighbours_groupID[id_grupo_actual*MAX_VECINOS + i] == vecino_siguiente){
			neighbour_number = i;
		}
	}

	return neighbour_number;
}

int FindNeighbour(int origin, int goal){
	
	int neighbour_number=-1;

	for(int i = 0; i< neighbours_number[origin]; i++){		
		if(neighbours_groupID[origin*MAX_VECINOS + i] == goal){
			neighbour_number = i;
		}
	}

	return neighbour_number;
}

void GetRailsToGroup(int neighbour_number, std::vector<xyLoc> &path){
				
	while(map[GetIndex(actual_point)] == id_grupo_actual){
		ReadRails(neighbours_number[id_grupo_actual], neighbour_number, actual_point, path);		
		actual_point = path.back();
	}
}

void GetRailsInFinalGroup(int neighbour_number, xyLoc goal, std::vector<xyLoc> &path){
	
	xyLoc aux_back = goal;
	
	path.push_back(goal);
	while(map[GetIndex(aux_back)] == id_grupo_actual){
		ReadRails(neighbours_number[id_grupo_actual], neighbour_number, aux_back, path);
		aux_back = path.back();
	}
}

bool SearchPathToGroup(int goalGroup, std::vector<xyLoc> &path){
	std::deque<xyLoc> q;
	visited.assign(basic_width*basic_height, 0);
	visited[GetIndex(actual_point)] = 1;
	q.push_back(actual_point);
	xyLoc next;
	int index_succ;
	while (q.empty() == false)
    {
		next = q.front();
		q.pop_front();
		GetSuccessors(next, succ);
		for (unsigned int x = 0; x < succ.size(); x++)
		{
			index_succ = GetIndex(succ[x]);

			if(map[index_succ] == id_grupo_actual){

				if (visited[index_succ] == false){
				
					visited[index_succ] = visited[GetIndex(next)]+1;
									
					q.push_back(succ[x]);				
				}
			}else if(map[index_succ] == goalGroup){	
				visited[index_succ] = visited[GetIndex(next)]+1;		

				actual_point = succ[x];
				ExtractPath(actual_point, path);			
				return true;
			}
		}
    }
	return false;
}

bool SearchPathToPoint(xyLoc goal, std::vector<xyLoc> &path){
	std::deque<xyLoc> q;
	visited.assign(basic_width*basic_height, 0);
	visited[GetIndex(actual_point)] = 1;
	q.push_back(actual_point);
	xyLoc next;
	int index_succ;
	while (q.empty() == false)
    {
		next = q.front();
		q.pop_front();
		GetSuccessors(next, succ);
		for (unsigned int x = 0; x < succ.size(); x++)
		{
			index_succ = GetIndex(succ[x]);

			if(map[index_succ] == id_grupo_actual){

				if (visited[index_succ] == false){
				
					visited[index_succ] = visited[GetIndex(next)]+1;
				
					if (succ[x].x == goal.x && succ[x].y == goal.y)
					{
						ExtractPath(goal, path);										
						return true;
					}					
						
					q.push_back(succ[x]);													
				}
			}
		}
    }
	return false;
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path){
	
	if(path.empty() == true){				
		path.push_back(s);
		actual_point = s;
		id_grupo_actual = map[GetIndex(actual_point)];
		id_grupo_meta = map[GetIndex(g)];	
		id_last_grupo = id_grupo_actual;
		railes_ultimo = false;

	}

	if(actual_point.x == g.x && actual_point.y == g.y){
		return true;
	}

	int num_vecino;

	std::vector<xyLoc> pathPartial;

	if(group_distances[id_grupo_actual*(TOPE+1)+id_grupo_meta] > 0){

		id_last_grupo = id_grupo_actual;

		num_vecino = FindNextNeighbour(actual_point, g);

		if(neighbours_number[id_grupo_actual] <= 16 || num_vecino < 16 ){ //TODO
			GetRailsToGroup(num_vecino, pathPartial);
			path.insert(path.end(), pathPartial.begin(), pathPartial.end());			
			railes_ultimo = true;
		}else{			
			SearchPathToGroup(neighbours_groupID[id_grupo_actual*MAX_VECINOS+num_vecino], pathPartial);
			path.insert(path.end(), pathPartial.begin(), pathPartial.end());
			railes_ultimo = false;
		}	

		id_grupo_actual = map[GetIndex(actual_point)];	

		return (actual_point.x == g.x && actual_point.y == g.y);			
	}
	
	bool end_rails = false;

	if(id_grupo_actual != id_last_grupo){	

		num_vecino = FindNeighbour(id_grupo_actual, id_last_grupo);

		if(neighbours_number[id_grupo_actual] <= 16 || num_vecino < 16){ 

			std::vector<xyLoc> pathPartialFinal;
			GetRailsInFinalGroup(num_vecino, g, pathPartialFinal);	
			if(railes_ultimo){
				end_rails = true;
				pathPartialFinal.pop_back();
				pathPartialFinal.pop_back();

				std::reverse(pathPartialFinal.begin(), pathPartialFinal.end());
				path.insert(path.end(), pathPartialFinal.begin(), pathPartialFinal.end());
			}else{
				if((int)pathPartialFinal.size()>2){

					end_rails = true;
					pathPartialFinal.pop_back();
					pathPartialFinal.pop_back();

					SearchPathToPoint(pathPartialFinal.back(), pathPartial);
					path.insert(path.end(), pathPartial.begin(), pathPartial.end());				

					pathPartialFinal.pop_back();

					std::reverse(pathPartialFinal.begin(), pathPartialFinal.end());
					path.insert(path.end(), pathPartialFinal.begin(), pathPartialFinal.end());
				}
			}			
		}
	}

	if(end_rails == false){	
		SearchPathToPoint(g, pathPartial);
		path.insert(path.end(), pathPartial.begin(), pathPartial.end());
	}

	return true;
}


void GetSuccessors(xyLoc s, std::vector<xyLoc> &neighbors){
	int width = basic_width;
	int height = basic_height;

	neighbors.resize(0);
  
	xyLoc next;
	xyLoc diagonal_1;
	xyLoc diagonal_2;

	next = s;
	next.y--;
	if (next.y >= 0 && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.x--;
	if (next.x >= 0 && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.x++;
	if (next.x < width && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.y++;
	if (next.y < height && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x--;
	next.y--;
	diagonal_1.x--;
	diagonal_2.y--;
	if(next.x >= 0 && next.y >= 0 && map[GetIndex(next)] > 0 && map[GetIndex(diagonal_1)] > 0 && map[GetIndex(diagonal_2)] > 0){
		neighbors.push_back(next);
	}

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x++;
	next.y--;
	diagonal_1.x++;
	diagonal_2.y--;
	if(next.x < width && next.y >= 0 && map[GetIndex(next)] > 0 && map[GetIndex(diagonal_1)] > 0 && map[GetIndex(diagonal_2)] > 0){
		neighbors.push_back(next);
	}

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x--;
	next.y++;
	diagonal_1.x--;
	diagonal_2.y++;
	if(next.x >= 0 && next.y < height && map[GetIndex(next)] > 0 && map[GetIndex(diagonal_1)] > 0 && map[GetIndex(diagonal_2)] > 0){
		neighbors.push_back(next);
	}
		
	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x++;
	next.y++;
	diagonal_1.x++;
	diagonal_2.y++;
	if(next.x < width && next.y < height && map[GetIndex(next)] > 0 && map[GetIndex(diagonal_1)] > 0 && map[GetIndex(diagonal_2)] > 0){
		neighbors.push_back(next);
	}
}

void GetSuccessors_cardinals(xyLoc s, std::vector<xyLoc> &neighbors){
	int width = basic_width;
	int height = basic_height;
	neighbors.resize(0);
  
	xyLoc next;

	next = s;
	next.y--;
	if (next.y >= 0 && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.x--;
	if (next.x >= 0 && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.x++;
	if (next.x < width && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}	

	next = s;
	next.y++;
	if (next.y < height && map[GetIndex(next)] > 0){
		neighbors.push_back(next);
	}
}

void GetSuccessors_diagonals(xyLoc s, std::vector<xyLoc> &neighbors){
	int width = basic_width;
	int height = basic_height;
	neighbors.resize(0);
  
	xyLoc next;
	xyLoc diagonal_1;
	xyLoc diagonal_2;

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x--;
	next.y--;
	diagonal_1.x--;
	diagonal_2.y--;
	if(next.x >= 0 && next.y >= 0 && map[GetIndex(next)]> 0 && map[GetIndex(diagonal_1)]> 0 && map[GetIndex(diagonal_2)]> 0){
		neighbors.push_back(next);
	}

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x++;
	next.y--;
	diagonal_1.x++;
	diagonal_2.y--;
	if(next.x < width && next.y >= 0 && map[GetIndex(next)]> 0 && map[GetIndex(diagonal_1)]> 0 && map[GetIndex(diagonal_2)]> 0){
		neighbors.push_back(next);
	}

	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x--;
	next.y++;
	diagonal_1.x--;
	diagonal_2.y++;
	if(next.x >= 0 && next.y < height && map[GetIndex(next)]> 0 && map[GetIndex(diagonal_1)]> 0 && map[GetIndex(diagonal_2)]> 0){
		neighbors.push_back(next);
	}
		
	next = s;
	diagonal_1 = s;
	diagonal_2 = s;
	next.x++;
	next.y++;
	diagonal_1.x++;
	diagonal_2.y++;
	if(next.x < width && next.y < height && map[GetIndex(next)]> 0 && map[GetIndex(diagonal_1)]> 0 && map[GetIndex(diagonal_2)]> 0){
		neighbors.push_back(next);
	}
}

void ExtractPath(xyLoc end, std::vector<xyLoc> &finalPath){	
	int currCost = visited[GetIndex(end)];
	int succ_index=0;
	finalPath.push_back(end);	
	while (currCost != 2)
    {    	
		GetSuccessors(finalPath.back(), succ);
		for (unsigned int x = 0; x < succ.size(); x++)
		{

			if (visited[GetIndex(succ[x])] == currCost-1)
			{
				succ_index = x;
				break;								
			}
		}

		finalPath.push_back(succ[succ_index]);
		currCost = visited[GetIndex(succ[succ_index])];
    }    

	std::reverse(finalPath.begin(), finalPath.end());	
}

int GetIndex(xyLoc s){

	return s.y*basic_width+s.x;
}


int ManhattanDistance(xyLoc a, xyLoc b){

	return abs(a.x - b.x) + abs(a.y - b.y);
}

//

int function_Manhattan_Steps(xyLoc a, xyLoc b, int currCost){

	return currCost - 1 + ManhattanDistance(a,b);
}