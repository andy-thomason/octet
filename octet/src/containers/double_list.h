////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// map strings to value_t.
//
// Double-linked list class
//
// example:
//
//   double_list<int> my_list;
//   my_list.push_back(1);
//   my_list.push_back(2);
//   my_list.push_back(3);
//
//   for (auto i = my_list.begin(); i != my_list.end(); ++i) {
//     printf("%d\n", *i);
//   }
// 
namespace octet {
  template <class item, class allocator_t=allocator> class double_list {
    struct double_list_head {
      // this makes new and delete use the allocator
      void *operator new(size_t size) {
        return allocator_t::malloc(size);
      }
      void operator delete(void *ptr, size_t size) {
        return allocator_t::free(ptr, size);
      }
      double_list_head *next;
      double_list_head *prev;
    };

    struct double_list_node : double_list_head {
      item item_;
      double_list_node(const item &new_item) { item_ = new_item; }
    };
  
    double_list_head head;
  
    /*void verify() {
      int n = 0;
      printf("+\n");
      for (double_list_head *ptr = head.next; ptr != &head; ptr = ptr->next) {
        assert(ptr->prev->next == ptr);
        assert(ptr->next->prev == ptr);
        printf("%p\n", ptr);
        n++;
      }
      printf("-\n");
      for (double_list_head *ptr = head.prev; ptr != &head; ptr = ptr->prev) {
        assert(ptr->prev->next == ptr);
        assert(ptr->next->prev == ptr);
        printf("%p\n", ptr);
        n--;
      }
      assert(n == 0);
    }*/
  
  public:
    double_list() {
      head.next = head.prev = &head;
    }

    ~double_list() {
      for (double_list_head *ptr = head.next, *next; ptr != &head; ptr = next) {
        next = ptr->next;
        // must return it to the correct pool!
        delete (double_list_node*)ptr;
      }
    }

    class iterator {
      double_list_node *node;
      friend class double_list;
    public:
      iterator(double_list_node *node) { this->node = node; }
      item *operator ->() { return &node->item_; }
      item &operator *() { return node->item_; }
      bool operator != (const iterator &rhs) const { return node != rhs.node; }
      iterator &operator++() { node = (double_list_node *)node->next; return *this; }
      iterator &operator--() { node = (double_list_node *)node->prev; return *this; }
    };

    iterator begin() {
      return iterator( (double_list_node*)head.next );
    }

    iterator end() {
      return iterator( (double_list_node*)&head );
    }
  
    iterator insert(iterator it, const item &new_item) {
      double_list_node *new_node = new double_list_node(new_item);
      //printf("insert %p at %p\n", new_node, it.node);
      new_node->next = it.node;
      new_node->prev = it.node->prev;
      new_node->prev->next = new_node;
      new_node->next->prev = new_node;
      //verify();
      return iterator(new_node);
    }

    // you can erase one element from the node
    iterator erase(iterator it) {
      double_list_head tmp = *it.node;
      tmp.next->prev = tmp.prev;
      tmp.prev->next = tmp.next;
      delete it.node;
      //verify();
      return iterator((double_list_node *)tmp.prev);
    }
  
    void push_back(const item &new_item) {
      double_list_node *new_node = new double_list_node(new_item);
      head.prev->next = new_node;
      new_node->next = &head;
      new_node->prev = head.prev;
      head.prev = new_node;
    }
  };
}
