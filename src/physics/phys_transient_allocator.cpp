#include "phys_transient_allocator.h"
#include "phys_mem_new.h"

void phys_transient_allocator::reset()
{
    phys_transient_allocator::block_header *m_first_block; // eax
    phys_slot_pool *m_slot_pool; // ebx
    phys_transient_allocator::block_header *m_next_block; // esi

    m_first_block = this->m_first_block;
    m_slot_pool = (phys_slot_pool *)this->m_slot_pool;
    if ( this->m_first_block )
    {
        do
        {
            m_next_block = m_first_block->m_next_block;
            PSP_FREE(m_slot_pool, (unsigned __int8 *)m_first_block);
            m_first_block = m_next_block;
        }
        while ( m_next_block );
    }
    this->m_first_block = 0;
    this->m_cur = 0;
    this->m_end = 0;
    this->m_total_memory_allocated = 0;
}
void __thiscall phys_transient_allocator::reset_to_state(const phys_transient_allocator::allocator_state *as)
{
    phys_transient_allocator::block_header *m_first_block; // eax
    phys_transient_allocator::block_header *m_next_block; // esi
    phys_slot_pool *slot_pool; // [esp+8h] [ebp-4h]

    slot_pool = (phys_slot_pool *)this->m_slot_pool;
    m_first_block = this->m_first_block;
    if ( this->m_first_block != as->m_first_block )
    {
        do
        {
            m_next_block = m_first_block->m_next_block;
            PSP_FREE(slot_pool, (unsigned __int8 *)m_first_block);
            m_first_block = m_next_block;
        }
        while ( m_next_block != as->m_first_block );
    }
    *(phys_transient_allocator::allocator_state *)&this->m_first_block = *as;
}

void __thiscall phys_transient_allocator::resize()
{
    phys_slot_pool *m_slot_pool; // eax
    char *v3; // eax

    m_slot_pool = (phys_slot_pool *)this->m_slot_pool;
    if ( !m_slot_pool )
    {
        m_slot_pool = GET_PHYS_SLOT_POOL(0x4000u, 4u);
        this->m_slot_pool = m_slot_pool;
    }
    v3 = PSP_ALLOC(m_slot_pool);
    if ( v3 )
    {
        *(unsigned int *)v3 = 0x4000;
        *((unsigned int *)v3 + 1) = 4;
        *((unsigned int *)v3 + 2) = (unsigned int)this->m_first_block;
        this->m_first_block = (phys_transient_allocator::block_header *)v3;
        this->m_total_memory_allocated += 0x4000;
        this->m_cur = v3 + 12;
        this->m_end = v3 + 0x4000;
    }
}

int phys_transient_allocator::mt_allocate_internal(int size, int alignment)
{
    char *cur; // [esp+Ch] [ebp-4h]

    do
    {
        cur = this->m_cur;
        if ((char *)(size + (~(alignment - 1) & (unsigned int)&cur[alignment - 1])) > this->m_end)
            return 0;
    } while ((char *)_InterlockedCompareExchange(
        (volatile unsigned __int32 *)&this->m_cur,
        size + (~(alignment - 1) & (unsigned int)&cur[alignment - 1]),
        (signed __int32)cur) != cur);
    return ~(alignment - 1) & (unsigned int)&cur[alignment - 1];
}

void *__thiscall phys_transient_allocator::allocate(
    int size,
    int alignment,
    int no_error,
    const char *error_msg)
{
    int v7; // [esp+4h] [ebp-14h]
    int v8; // [esp+Ch] [ebp-Ch]
    void *ptr; // [esp+14h] [ebp-4h]

    transient_allocator_update_largest_size();
    v8 = ~(alignment - 1) & (int)&this->m_cur[alignment - 1];
    if ((char *)(size + v8) <= this->m_end)
    {
        this->m_cur = (char *)(size + v8);
        ptr = (void *)v8;
    }
    else
    {
        ptr = 0;
    }
    if (!ptr)
    {
        //phys_transient_allocator::resize();
        this->resize();
        v7 = ~(alignment - 1) & (int)&this->m_cur[alignment - 1];
        if ((char *)(size + v7) <= this->m_end)
        {
            this->m_cur = (char *)(size + v7);
            ptr = (void *)v7;
        }
        else
        {
            ptr = 0;
        }
        if (!ptr
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                79,
                "ptr",
                "transient allocation too large, increase block_size."))
        {
            __debugbreak();
        }
    }
    if (!ptr
        && !no_error
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
            81,
            "ptr || no_error",
            error_msg))
    {
        __debugbreak();
    }
    return ptr;
}

void *__thiscall phys_transient_allocator::mt_allocate(
    int size,
    int alignment,
    int no_error,
    const char *error_msg)
{
    int v7; // [esp+Ch] [ebp-40h]
    int v8; // [esp+14h] [ebp-38h]
    void *ptr; // [esp+48h] [ebp-4h]

    transient_allocator_update_largest_size();
    //minspec_read_write_mutex::ReadLock(&this->m_mutex);
    this->m_mutex.ReadLock();
    ptr = (void *)phys_transient_allocator::mt_allocate_internal(size, alignment);
    //minspec_read_write_mutex::ReadUnlock(&this->m_mutex);
    this->m_mutex.ReadUnlock();
    if (!ptr)
    {
        //minspec_read_write_mutex::WriteLock(&this->m_mutex);
        this->m_mutex.WriteLock();
        v8 = ~(alignment - 1) & (int)&this->m_cur[alignment - 1];
        if ((char *)(size + v8) <= this->m_end)
        {
            this->m_cur = (char *)(size + v8);
            ptr = (void *)v8;
        }
        else
        {
            ptr = 0;
        }
        if (!ptr)
        {
            phys_transient_allocator::resize();
            v7 = ~(alignment - 1) & (int)&this->m_cur[alignment - 1];
            if ((char *)(size + v7) <= this->m_end)
            {
                this->m_cur = (char *)(size + v7);
                ptr = (void *)v7;
            }
            else
            {
                ptr = 0;
            }
            if (!ptr
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                    99,
                    "ptr",
                    "transient allocation too large, increase block_size."))
            {
                __debugbreak();
            }
        }
        //minspec_read_write_mutex::WriteUnlock(&this->m_mutex);
        this->m_mutex.WriteUnlock();
    }
    if (!ptr
        && !no_error
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
            103,
            "ptr || no_error",
            error_msg))
    {
        __debugbreak();
    }
    return ptr;
}