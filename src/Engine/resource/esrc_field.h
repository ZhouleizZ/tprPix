/*
 * ========================= esrc_field.h ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.04.19
 *                                        MODIFY --
 * ----------------------------------------------------------
 */
#ifndef TPR_ESRC_FIELD_H
#define TPR_ESRC_FIELD_H

//-------------------- CPP --------------------//
#include <utility> //- pair
#include <memory>

//-------------------- Engine --------------------//
#include "MapField.h"
#include "fieldKey.h"
#include "MapFieldData_In_ChunkCreate.h"

namespace esrc {//------------------ namespace: esrc -------------------------//

void init_fields();

void atom_try_to_insert_and_init_the_field_ptr( const IntVec2 &fieldMPos_ );
void atom_field_reflesh_min_and_max_altis(fieldKey_t fieldKey_, const MapAltitude &alti_ );

void atom_field_set_nodeAlti_2( fieldKey_t fieldKey_, const std::vector<std::unique_ptr<MemMapEnt>> &chunkMapEnts_ );

std::unique_ptr<MapFieldData_In_ChunkCreate> atom_get_mapFieldData_in_chunkCreate( fieldKey_t fieldKey_ );
void atom_create_a_go_in_field( fieldKey_t fieldKey_ );

void atom_erase_all_fields_in_chunk( const IntVec2 &chunkMPos_ );

//-- tmp，仅用于 debug，在未来，要被删除
const MapField &atom_get_field( fieldKey_t fieldKey_ );   // tmp......


}//---------------------- namespace: esrc -------------------------//
#endif
