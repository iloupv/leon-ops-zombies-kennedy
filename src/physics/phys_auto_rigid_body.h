#pragma once
#include "rigid_body.h"
#include "phys_gjk_collision_detection.h"

struct centity_s;

struct auto_rigid_body // sizeof=0x1C
{
    // for avl tree access 
    static inline bool less(const centity_s *key, const auto_rigid_body *node)
    {
        return key < node->cent;
    }
    static inline bool less(const auto_rigid_body *node, const centity_s *key)
    {
        return node->cent < key;
    }
    static inline bool less(const auto_rigid_body *a, const auto_rigid_body *b)
    {
        return a->cent < b->cent;
    }
    static inline bool equals(const auto_rigid_body *node, const centity_s *key)
    {
        return node->cent == key;
    }
    static inline void set_key(auto_rigid_body *node, const centity_s *key)
    {
        node->cent = key;
    }

    user_rigid_body *rb;
    const centity_s *cent;
    int frame_count;
    auto_rigid_body *next;
    phys_inplace_avl_tree_node<auto_rigid_body> m_avl_tree_node;

    static void add(const centity_s *cent, gjk_physics_collision_visitor *collision_visitor, int mask);
    static void remove_ent(const centity_s *cent);
    static user_rigid_body *ent_has_auto_rigid_body(const centity_s *cent);
    static void update();
};

void __cdecl fixup_wheel_constraints_arb(auto_rigid_body *arb);