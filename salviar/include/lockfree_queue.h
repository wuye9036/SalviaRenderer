#ifndef SALVIAR_LOCKFREE_QUEUE_H
#define SALVIAR_LOCKFREE_QUEUE_H

#include "salviar/include/atomic.h"

template <typename T>
class lockfree_queue
{
private:
	struct node_t
	{
		T value;
		atomic<node_t*> next;
	};

public:
	lockfree_queue()
	{
		node_t* node = new node_t;
		node->next = NULL;
		head_ = tail_ = atomic<node_t*>(node);
	}

	lockfree_queue(const lockfree_queue& rhs)
	{
		this->copy_from(rhs);
	}

	~lockfree_queue()
	{
		atomic<node_t*> node = head_;
		while (node.value() != NULL)
		{
			atomic<node_t*> next = node.value()->next;
			delete node.value();
			node = next;
		}
	}

	lockfree_queue& operator=(const lockfree_queue& rhs)
	{
		if (this != &rhs)
		{
			this->copy_from(rhs);
		}

		return *this;
	}

	void copy_from(const lockfree_queue& rhs)
	{
		node_t* node = new node_t;
		node->next = NULL;
		head_ = tail_ = atomic<node_t*>(node);

		atomic<node_t*> rhs_node = rhs.head_.value()->next;
		while (rhs_node.value() != NULL)
		{
			this->enqueue(rhs_node.value()->value);
			rhs_node = rhs_node.value()->next;
		}
	}

	bool empty() const
	{
		return (head_ == tail_);
	}

	void enqueue(const T& value)
	{
		node_t* node = new node_t;
		node->value = value;
		node->next = NULL;

		for (;;)
		{
			atomic<node_t*> tail = tail_;
			atomic<node_t*> next = tail.value()->next;
			if (tail == tail_)
			{
				if (NULL == next.value())
				{
					if (tail.value()->next.cas(next.value(), node))
					{
						tail_.cas(tail.value(), node);
						break;
					}
				}
				else
				{
					tail_.cas(tail.value(), next.value());
				}
			}
		}
	}

	bool dequeue(T& ret)
	{
		for (;;)
		{
			atomic<node_t*> head = head_;
			atomic<node_t*> tail = tail_;
			atomic<node_t*> next = head.value()->next;
			if (head == head_)
			{
				if (head == tail)
				{
					if (NULL == next.value())
					{
						return false;
					}
					tail_.cas(tail.value(), next.value());
				}
				else
				{
					ret = next.value()->value;
					if (head_.cas(head.value(), next.value()))
					{
						delete head.value();
						return true;
					}
				}
			}
		}
	}

	// Not thread safe
	template <typename iter_type>
	void dequeue_all(iter_type begin)
	{
		atomic<node_t*> head = head_;
		atomic<node_t*> tail = tail_;
		atomic<node_t*> next = head.value()->next;
		while (next != NULL)
		{
			*begin = next.value()->value;
			++ begin;
			next = next.value()->next;
		}
	}

private:
	atomic<node_t*> head_, tail_;
};

#endif		// SALVIAR_LOCKFREE_QUEUE_H
