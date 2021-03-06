#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

/* In the first fit algorithm, the allocator keeps a list of free blocks (known as the free list) and,
   on receiving a request for memory, scans along the list for the first block that is large enough to
   satisfy the request. If the chosen block is significantly larger than that requested, then it is 
   usually split, and the remainder added to the list as another free block.
   Please see Page 196~198, Section 8.2 of Yan Wei Ming's chinese book "Data Structure -- C programming language"
*/
// LAB2 EXERCISE 1: YOUR CODE
// you should rewrite functions: default_init,default_init_memmap,default_alloc_pages, default_free_pages.
/*
 * Details of FFMA
 * (1) Prepare: In order to implement the First-Fit Mem Alloc (FFMA), we should manage the free mem block use some list.
 *              The struct free_area_t is used for the management of free mem blocks. At first you should
 *              be familiar to the struct list in list.h. struct list is a simple doubly linked list implementation.
 *              You should know howto USE: list_init, list_add(list_add_after), list_add_before, list_del, list_next, list_prev
 *              Another tricky method is to transform a general list struct to a special struct (such as struct page):
 *              you can find some MACRO: le2page (in memlayout.h), (in future labs: le2vma (in vmm.h), le2proc (in proc.h),etc.)
 * (2) default_init: you can reuse the  demo default_init fun to init the free_list and set nr_free to 0.
 *              free_list is used to record the free mem blocks. nr_free is the total number for free mem blocks.
 * (3) default_init_memmap:  CALL GRAPH: kern_init --> pmm_init-->page_init-->init_memmap--> pmm_manager->init_memmap
 *              This fun is used to init a free block (with parameter: addr_base, page_number).
 *              First you should init each page (in memlayout.h) in this free block, include:
 *                  p->flags should be set bit PG_property (means this page is valid. In pmm_init fun (in pmm.c),
 *                  the bit PG_reserved is setted in p->flags)
 *                  if this page  is free and is not the first page of free block, p->property should be set to 0.
 *                  if this page  is free and is the first page of free block, p->property should be set to total num of block.
 *                  p->ref should be 0, because now p is free and no reference.
 *                  We can use p->page_link to link this page to free_list, (such as: list_add_before(&free_list, &(p->page_link)); )
 *              Finally, we should sum the number of free mem block: nr_free+=n
 * (4) default_alloc_pages: search find a first free block (block size >=n) in free list and reszie the free block, return the addr
 *              of malloced block.
 *              (4.1) So you should search freelist like this:
 *                       list_entry_t le = &free_list;
 *                       while((le=list_next(le)) != &free_list) {
 *                       ....
 *                 (4.1.1) In while loop, get the struct page and check the p->property (record the num of free block) >=n?
 *                       struct Page *p = le2page(le, page_link);
 *                       if(p->property >= n){ ...
 *                 (4.1.2) If we find this p, then it' means we find a free block(block size >=n), and the first n pages can be malloced.
 *                     Some flag bits of this page should be setted: PG_reserved =1, PG_property =0
 *                     unlink the pages from free_list
 *                     (4.1.2.1) If (p->property >n), we should re-caluclate number of the the rest of this free block,
 *                           (such as: le2page(le,page_link))->property = p->property - n;)
 *                 (4.1.3)  re-caluclate nr_free (number of the the rest of all free block)
 *                 (4.1.4)  return p
 *               (4.2) If we can not find a free block (block size >=n), then return NULL
 * (5) default_free_pages: relink the pages into  free list, maybe merge small free blocks into big free blocks.
 *               (5.1) according the base addr of withdrawed blocks, search free list, find the correct position
 *                     (from low to high addr), and insert the pages. (may use list_next, le2page, list_add_before)
 *               (5.2) reset the fields of pages, such as p->ref, p->flags (PageProperty)
 *               (5.3) try to merge low addr or high addr blocks. Notice: should change some pages's p->property correctly.
 */
free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static void
default_init(void)
{
  list_init(&free_list);
  nr_free = 0;
}

static void
default_init_memmap(struct Page *base, size_t n)
{
  assert(n > 0);
  struct Page *p = base;
  /*
    遍历所有空闲物理页的Page结构，
    将Page结构的描述空闲块的数目的成员变量置零
    （因此该成员变量只有在整个空闲块的第一个Page中才有意义），
    然后清空这些物理页的引用计数，然后通过设置flags的位的方式将其标记为空闲，
    */
  for (; p != base + n; p++)
  {
    assert(PageReserved(p)); //Page数据结构的成员变量property用来记录某连续内存空闲块的大小（即地址连续的空闲页的个数）
    p->flags = p->property = 0;
    set_page_ref(p, 0);
  }
  /*
    接下来对空闲块的第一个页的Page结构进行初始化，
    具体实现为将其表示空闲块大小的成员变量设置为作为参数传入的空闲块大小（单位为页），
    然后更新存储所有空闲页数量的全局变量，然后将这个空闲块插入到空闲内存块链表中
    （只需要将第一个Page的page_link插入即可）
    */
  base->property = n;
  SetPageProperty(base);
  nr_free += n;
  list_add(&free_list, &(base->page_link));
}

/*
该函数的具体功能为分配指定页数的连续空闲物理空间，
并且将第一页的Page结构的指针作为结果返回；该函数的具体实现方式如下：
*/
static struct Page *
default_alloc_pages(size_t n)
{
  //对参数进行合法性检查，并且查询总的空闲物理页数目是否足够进行分配，如果不足够进行分配，直接返回NULL，表示分配失败；
  assert(n > 0);
  if (n > nr_free)
  {
    return NULL;
  }
  struct Page *page = NULL;
  list_entry_t *le, *len; //当前list entry和下一个list entry
  le = &free_list;
  /*
    从头开始遍历保存空闲物理内存块的链表(按照物理地址的从小到大顺序)，
    如果找到某一个连续内存块的大小不小于当前需要的连续内存块大小，
    则说明可以进行成功分配（选择第一个遇到的满足条件的空闲内存块来完成内存分配）
    如果该内存块的大小大于需要的内存大小，则将空闲内存块分裂成两块，
    物理地址较小的一块分配出来进行使用（大小恰好为需要的物理内存的大小），
    而物理地址较大的那一块重新进行初始化（包括对第一个Page中表示空闲块大小的成员变量进行设置，
    其应当设置为原先的空闲块大小减掉分配掉的大小，
    以及将这个分裂出来的空闲块插入到空闲块链表中（该链表中的空闲块按照物理地址从小到大排序））；
    如果原先的空闲块大小刚好等于需要的内存大小，则没有比较进行分裂；
    于此同时，对分配出去的物理内存的每一个的描述信息（即对应的Page结构）进行初始化，
    具体为修改flags成员变量来将这些Page标记为非空闲，最后将原始空闲块在空闲块链表中删除掉，
    并且更新表示总空闲页数量的全局变量；最后用于表示分配到的物理内存的Page结构指针返回。
    */
  while ((le = list_next(le)) != &free_list)
  {
    struct Page *p = le2page(le, page_link);
    if (p->property >= n)
    { //如果找到一个足够大的空闲块，
      int i;
      for (i = 0; i < n; i++)
      { //依次向后遍历n个page，
        struct Page *pp = le2page(le, page_link);
        SetPageReserved(pp);   //并将这些page标记为已占用
        ClearPageProperty(pp); //清空property位，表示这些page不是头page

        len = list_next(le);
        list_del(le); //从空闲列表删除
        le = len;
      }
      if (p->property > n)
      { //如果还有剩余的，就修改page头的剩余量
        (le2page(le, page_link))->property = p->property - n;
      }
      ClearPageProperty(p); //头page为0表示已经被分配
      SetPageReserved(p);   //把page标记为已占用
      nr_free -= n;
      return p;
      //最后结果是满足大小的若干个page从空闲列表中删去，并且设置相应标记位
    }
  }
  return NULL;
}
/*
释放指定的某一物理页开始的若干个连续物理页，
并且完成first-fit算法中需要的若干信息的维护，
*/
static void
default_free_pages(struct Page *base, size_t n)
{
  //确定是否合法，page是否真的被占用
  assert(n > 0);
  assert(PageReserved(base));

  //从free_list开始找，找到合适插入base的地方（找到地址比base高的块）为止
  list_entry_t *le = &free_list;
  struct Page *p;
  while ((le = list_next(le)) != &free_list)
  {
    p = le2page(le, page_link);
    if (p > base)
    {
      break;
    }
  }
  //插入算上base一共n个的空闲块
  //list_add_before(le, base->page_link);
  for (p = base; p < base + n; p++)
  {
    list_add_before(le, &(p->page_link));
  }
  //将base标记设置为空闲
  base->flags = 0;
  set_page_ref(base, 0);
  ClearPageProperty(base);
  SetPageProperty(base);
  base->property = n;

  //此时le为base后面的page。
  p = le2page(le, page_link);
  //和后面的相邻空闲块进行合并
  if (base + n == p)
  {
    base->property += p->property;
    p->property = 0;
  }
  //和前面的相邻块合并
  le = list_prev(&(base->page_link));
  p = le2page(le, page_link);
  if (le != &free_list && p == base - 1)
  {
    while (le != &free_list)
    {
      //如果是空闲的就把base合并到前面去
      if (p->property)
      {
        p->property += base->property;
        base->property = 0;
        break;
      }
      le = list_prev(le);
      p = le2page(le, page_link);
    }
  }

  nr_free += n;
  return;
}

static size_t
default_nr_free_pages(void)
{
  return nr_free;
}

static void
basic_check(void)
{
  struct Page *p0, *p1, *p2;
  p0 = p1 = p2 = NULL;
  assert((p0 = alloc_page()) != NULL);
  assert((p1 = alloc_page()) != NULL);
  assert((p2 = alloc_page()) != NULL);

  assert(p0 != p1 && p0 != p2 && p1 != p2);
  assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

  assert(page2pa(p0) < npage * PGSIZE);
  assert(page2pa(p1) < npage * PGSIZE);
  assert(page2pa(p2) < npage * PGSIZE);

  list_entry_t free_list_store = free_list;
  list_init(&free_list);
  assert(list_empty(&free_list));

  unsigned int nr_free_store = nr_free;
  nr_free = 0;

  assert(alloc_page() == NULL);

  free_page(p0);
  free_page(p1);
  free_page(p2);
  assert(nr_free == 3);

  assert((p0 = alloc_page()) != NULL);
  assert((p1 = alloc_page()) != NULL);
  assert((p2 = alloc_page()) != NULL);

  assert(alloc_page() == NULL);

  free_page(p0);
  assert(!list_empty(&free_list));

  struct Page *p;
  assert((p = alloc_page()) == p0);
  assert(alloc_page() == NULL);

  assert(nr_free == 0);
  free_list = free_list_store;
  nr_free = nr_free_store;

  free_page(p);
  free_page(p1);
  free_page(p2);
}

// LAB2: below code is used to check the first fit allocation algorithm (your EXERCISE 1)
// NOTICE: You SHOULD NOT CHANGE basic_check, default_check functions!
static void
default_check(void)
{
  int count = 0, total = 0;
  list_entry_t *le = &free_list;
  while ((le = list_next(le)) != &free_list)
  {
    struct Page *p = le2page(le, page_link);
    assert(PageProperty(p));
    count++, total += p->property;
  }
  assert(total == nr_free_pages());

  basic_check();

  struct Page *p0 = alloc_pages(5), *p1, *p2;
  assert(p0 != NULL);
  assert(!PageProperty(p0));

  list_entry_t free_list_store = free_list;
  list_init(&free_list);
  assert(list_empty(&free_list));
  assert(alloc_page() == NULL);

  unsigned int nr_free_store = nr_free;
  nr_free = 0;

  free_pages(p0 + 2, 3);
  assert(alloc_pages(4) == NULL);
  assert(PageProperty(p0 + 2) && p0[2].property == 3);
  assert((p1 = alloc_pages(3)) != NULL);
  assert(alloc_page() == NULL);
  assert(p0 + 2 == p1);

  p2 = p0 + 1;
  free_page(p0);
  free_pages(p1, 3);
  assert(PageProperty(p0) && p0->property == 1);
  assert(PageProperty(p1) && p1->property == 3);

  assert((p0 = alloc_page()) == p2 - 1);
  free_page(p0);
  assert((p0 = alloc_pages(2)) == p2 + 1);

  free_pages(p0, 2);
  free_page(p2);

  assert((p0 = alloc_pages(5)) != NULL);
  assert(alloc_page() == NULL);

  assert(nr_free == 0);
  nr_free = nr_free_store;

  free_list = free_list_store;
  free_pages(p0, 5);

  le = &free_list;
  while ((le = list_next(le)) != &free_list)
  {
    struct Page *p = le2page(le, page_link);
    count--, total -= p->property;
  }
  assert(count == 0);
  assert(total == 0);
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .check = default_check,
};
