#pragma once

struct phys_assert_info // sizeof=0x18
{                                       // XREF: .data:phys_assert_info pai_check_terrain_query_params/r
    volatile unsigned int m_hits_total_count;             // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+E5/r
                                        // phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+1C0/r ...
    volatile unsigned int m_hits_frame_count;             // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool):loc_83FE6C/r
                                        // phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+17D/o ...
    int m_max_hits_total;               // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+EB/r
                                        // phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+F3/r ...
    int m_max_hits_per_frame;           // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+101/r
                                        // phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+1DC/r ...
    bool m_use_warnings_only;           // XREF: phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+109/r
                                        // phys_heap_gjk_cache_system_avl_tree::get_gjk_cache_info(uint,uint,bool)+1E4/r ...
    // padding byte
    // padding byte
    // padding byte
    phys_assert_info *m_next;           // XREF: _dynamic_initializer_for__pai_check_terrain_query_params__+5/w

    phys_assert_info(int max_hits_total, int max_hits_per_frame, bool use_warnings_only);
    void frame_advance();
};

void __cdecl phys_set_debug_callback(void (__cdecl *debug_callback)(void *));
void __cdecl phys_exec_debug_callback(void *data);
void __cdecl PHYS_WARNING(const char *file, int line, const char *expr, const char *desc);
void __cdecl phys_assert_info_frame_advance_all();


static const char *SOLVER_MEMORY_ALLOCATOR_ERROR_MSG = "physics constraint solver OUT OF MEMORY.";
