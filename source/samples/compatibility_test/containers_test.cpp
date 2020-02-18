//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "containers_test.h"
//----------------------------------------------------------------------------//
ContainersTestUnit::ContainersTestUnit() : TestUnit("CONTAINERS")
{
	tasks.Add(new ArrayOpsTask);
	tasks.Add(new MapTask);
	tasks.Add(new TreeTask);
	tasks.Add(new AVLTreeTask);
	tasks.Add(new SharedPtrTask);
	tasks.Add(new ContainerTask);
	tasks.Add(new OptionalTask);
	tasks.Add(new ResultTask);
	tasks.Add(new PairTask);
}

//----------------------------------------------------------------------------//
ArrayOpsTask::ArrayOpsTask() : TestTask("Array ops")
{ }

void ArrayOpsTask::Execute()
{
	// array test, following calls can cause
	// throwing an error if something went wrong:
	ut::Array<int> arr;
	arr.Add(1);
	arr.Add(13);
	arr.Add(3);
	arr.Add(63);
	arr.Add(101);
	arr.Add(22);
	arr.Add(31);
	int a = arr[0] + arr[1];
	arr.Add(a);
	arr.PushForward(0);
	arr.Insert(arr.Begin() + 2, 2);
	arr.Insert(4, 5);

	// test move constructor
	ut::Array<int> arr2;
	arr2 = ut::Move(arr);

	// test last element reference
	int last = arr2.GetLast();
	report.Print("testing array operations, final value must be 14: %i", last);
	if (last != 14)
	{
		failed_test_counter.Increment();
	}

	// test removing an element
	arr2.Remove(arr2.Begin() + 3);

	// test array concatenation
	report += ". Testing array concatenation:";
	ut::Array<int> iarr0;
	int ii = 0;
	iarr0.Add(0);
	iarr0.Add(1);
	ut::Array<int> iarr1;
	iarr0.Add(2);
	iarr0.Add(3);
	iarr0.Add(4);
	ut::Array<int> iarr2 = iarr0 + iarr1;
	iarr0 += iarr1;
	if (iarr2.GetNum() != 5 || iarr0.GetNum() != 5)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
	else if (iarr0[0] != 0 || iarr0[1] != 1 || iarr0[2] != 2 || iarr0[3] != 3 || iarr0[4] != 4)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
	else if (iarr2[0] != 0 || iarr2[1] != 1 || iarr2[2] != 2 || iarr2[3] != 3 || iarr2[4] != 4)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}

	// UniquePtr concatenation
	ut::Array< ut::UniquePtr<int> > uniqarr0;
	ut::Array< ut::UniquePtr<int> > uniqarr1;
	uniqarr0 += ut::Move(uniqarr1);

	// test reverse order iterator
	report += ". Testing array reverse iterator:";
	ut::Array<int>::Iterator riterator;
	for (riterator = arr2.Begin(ut::iterator::last); riterator > arr2.End(ut::iterator::first); --riterator)
	{
		int val = *riterator;
		int& val1 = *riterator;
		ut::String str;
		str.Print("%i ", val);
		report += str;
	}

	// test normal iterator
	report += ". Testing array forward iterator:";
	ut::Array<int>::Iterator iterator;
	for (iterator = arr2.Begin(ut::iterator::first); iterator < arr2.End(ut::iterator::last); ++iterator)
	{
		int val = *iterator;
		int& val1 = *iterator;
		ut::String str;
		str.Print("%i ", val);
		report += str;
	}
}

//----------------------------------------------------------------------------//
MapTask::MapTask() : TestTask("Map")
{ }

void MapTask::Execute()
{
	// map test, following calls can cause
	// throwing an error if something went wrong:
	ut::Map<int, ut::String> map;
	ut::String str1("__2");
	ut::String str2("__32");
	int ival = 32;
	map.Insert(24, "__24");
	map.Insert(2, str1);
	map.Insert(ival, str2);
	map.Add(ut::Pair<int, ut::String>(1, "__1"));
	map.Insert(55, "__55");
	map.Insert(4, "__4");
	map.Insert(3, "__3");
	map.Remove(4);
	map.Insert(5, "__5");
	map.Insert(43, "__43");
	map.Insert(60, "__60");

	// try to find one specific value by key
	report += "searching element by key \'55\'(should be \'__55\'): ";
	ut::Optional<ut::String&> find_result = map.Find(55);
	if (find_result)
	{
		// get the value
		ut::String& str = find_result.Get();
		report += str;
		if (str == "__55")
		{
			report += ". Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
	}

	// UniquePtr, compile-time check
	ut::Map<int, ut::UniquePtr<ut::String> > uniqmap0;
	ut::Map<int, ut::UniquePtr<ut::String> > uniqmap1(ut::Move(uniqmap0));
	ut::UniquePtr<ut::String> nstr0(new ut::String("str"));
	uniqmap1.Insert(0, ut::Move(nstr0));
	uniqmap1.Remove(0);

	// another way to find specific value by key
	report += ". Searching element by key \'4\'(should not be found): ";
	ut::Optional<ut::String&> find_result2 = map.Find(4);
	if (find_result2)
	{
		report += "somehow it was found. Fail.";
		failed_test_counter.Increment();
	}
	else
	{
		report += "not found. Success";
	}

	// test map iterators
	report += ". Testing iterator (should be __2): ";
	ut::Map<int, ut::String>::Iterator iterator = map.Begin();
	iterator += 1;
	ut::Pair<int, ut::String>& pair = *iterator;
	report += pair.second;
	if (pair.second == "__2")
	{
		report += ". Success.";
	}
	else
	{
		report += ". Fail.";
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
TreeTask::TreeTask() : TestTask("Tree")
{ }

void TreeTask::Execute()
{
	// tree test, following calls can cause
	// throwing an error if something went wrong:
	ut::Tree<ut::String> tree;
	tree.data = "0";
	report += "adding nodes: ";
	tree.Add("0_0");
	tree.Add("0_1");
	tree.Add("0_2");

	// test how many children
	if (tree.GetNumChildren() == 3)
	{
		bool success = true;
		for (size_t i = 0; i < 3; i++)
		{
			// trying to add inner nodes
			for (size_t j = 0; j < 3; j++)
			{
				ut::String val = ut::Print<ut::uint32>((ut::uint32)j);
				tree[i].Add(tree[i].data + "_" + val);
			}

			// validate inner nodes
			if (tree[i].GetNumChildren() != 3)
			{
				success = false;
				report += "Failed to add inner nodes. ";
				failed_test_counter.Increment();
			}
		}

		if (success)
		{
			report += "Success. ";
		}
	}
	else
	{
		report += "Failed to add outer nodes. ";
		failed_test_counter.Increment();
	}

	// test remove() functionality
	report += "Removing child node: ";
	ut::Array< ut::Tree<ut::String> >::Iterator leaf_iterator = tree.BeginLeaves();
	tree.Remove(leaf_iterator);

	// check the number of nodes after all manipulations
	report += "Calculating tree nodes: ";
	if (tree.GetNum() == 9)
	{
		report += "Success, ";
		report += ut::Print<ut::uint32>((ut::uint32)tree.GetNum());
		report += " nodes.";
	}
	else
	{
		report += "Fail, ";
		report += ut::Print<ut::uint32>((ut::uint32)tree.GetNum());
		report += " nodes.";
		failed_test_counter.Increment();
	}

	// test insert() functionality
	report += " Inserting nodes: ";
	if (tree.Insert(1, "inserted0"))
	{
		ut::Array< ut::Tree<ut::String> >::ConstIterator insert_iter = tree.BeginLeaves();
		insert_iter += 2;
		if (tree.Insert(insert_iter, "inserted1"))
		{
			report += "Success. ";
		}
		else
		{
			report += "Fail. ";
		}
	}
	else
	{
		report += "Fail. ";
		failed_test_counter.Increment();
	}

	// debug output: all leaves
	report += " Leaves output (forward): ";
	ut::Tree<ut::String>::Iterator forward_iter;
	for (forward_iter = tree.Begin(ut::iterator::first); forward_iter != tree.End(ut::iterator::last); ++forward_iter)
	{
		forward_iter->data += "!";
		report += forward_iter->data;
		report += " ";
	}

	// debug output: all leaves (reverse order)
	report += " Leaves output (backward): ";
	ut::Tree<ut::String>::ConstIterator back_iter;
	ut::Tree<ut::String>& cnode = tree;
	for (back_iter = cnode.Begin(ut::iterator::last); back_iter != cnode.End(ut::iterator::first); --back_iter)
	{
		report += back_iter->data;
		report += " ";
	}
}

//----------------------------------------------------------------------------//

AVLTreeTask::AVLTreeTask() : TestTask("AVL Tree")
{ }

void AVLTreeTask::Execute()
{
	// avltree test, following calls can cause
	// throwing an error if something went wrong:
	ut::AVLTree<int, ut::String> tree;
	tree.Insert(24, "__24");
	tree.Insert(2,  "__2");
	tree.Insert(32, "__32");
	tree.Insert(10, "__10");
	tree.Insert(15, "__15");
	tree.Insert(1,  "__1");
	tree.Insert(55, "__55");
	tree.Insert(4,  "__4");
	tree.Insert(127, "__127");
	tree.Insert(3,  "__3");
	tree.Insert(5,  "__5");
	tree.Insert(43, "__43");
	tree.Insert(60, "__60");
	tree.Insert(18, "__18");
	tree.Insert(22, "__22");
	tree.Insert(49, "__49");
	tree.Insert(29, "__29");
	tree.Insert(27, "__27");
	tree.Insert(25, "__25");

	// try to remove a leaf by key
	tree.Remove(32);
	tree.Remove(127);
	tree.Remove(3);
	tree.Remove(5);
	tree.Remove(24);

	// then insert some new values
	tree.Insert(26, "__26");
	tree.Insert(33, "__33");
	tree.Insert(128, "__128");
	tree.Insert(129, "__129");
	tree.Insert(26, "__26_1");
	tree.Insert(26, "__26_2");

	// try to find a specific value by key
	report += "searching element by key \'3\'(should be \'__3\'): ";
	ut::Optional<ut::String&> find_result = tree.Find(128);
	if (find_result)
	{
		ut::String& str = find_result.Get();
		report += str;
		if (str == "__128")
		{
			report += ". Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// check if deleted nodes were deleted
	/*
	report += "check deleted nodes: ";
	find_result = tree.Find(32);
	if (find_result)
	{
		report += ut::String("Error! node wasn't deleted properly");
		failed_test_counter.Increment();
		return;
	}
	else
	{
		report += "Success";
	}
	*/

	// iterate avl container forward
	report += ut::CRet() + "iterating forward: ";
	ut::AVLTree<int, ut::String>::ConstIterator riterator;
	int previous_key = 0;
	for (riterator = tree.Begin(ut::iterator::first); riterator != tree.End(ut::iterator::last); ++riterator)
	{
		const ut::AVLTree<int, ut::String>::Node& node = *riterator;
		report += node.value;

		// check balance
		ut::int8 balance = node.GetBalance();
		if (balance < -1 || balance > 1)
		{
			report += ut::String(" Error, invalid balance!\n");
			failed_test_counter.Increment();
			return;
		}

		int key = node.GetKey();
		if (previous_key > key)
		{
			report += ut::String(" Error, invalid order!\n");
			failed_test_counter.Increment();
			return;
		}
		previous_key = key;
	}

	// iterate avl container backward
	report += ut::CRet() + "iterating backward: ";
	ut::AVLTree<int, ut::String>::Iterator literator;
	previous_key = 100000;
	for (literator = tree.Begin(ut::iterator::last); literator != tree.End(ut::iterator::first); --literator)
	{
		ut::AVLTree<int, ut::String>::Node& node = *literator;
		report += node.value;

		int key = node.GetKey();
		if (previous_key < key)
		{
			report += ut::String(" Error, invalid order!\n");
			failed_test_counter.Increment();
			return;
		}
		previous_key = key;
	}

	// copy test
	report += ut::CRet() + "copy constructor: ";
	ut::AVLTree<int, ut::String> tree_copy0(tree);
	ut::Optional<ut::String&> copy0_find_result = tree_copy0.Find(128);
	if (copy0_find_result)
	{
		ut::String& str = copy0_find_result.Get();
		if (str == "__128")
		{
			report += ". Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// copy assignment test
	report += ut::CRet() + "copy assignment: ";
	ut::AVLTree<int, ut::String> tree_copy1;
	tree_copy1 = tree;
	ut::Optional<ut::String&> copy1_find_result = tree_copy1.Find(128);
	if (copy1_find_result)
	{
		ut::String& str = copy1_find_result.Get();
		if (str == "__128")
		{
			report += ". Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// move constructor test
#if CPP_STANDARD >= 2011
	report += ut::CRet() + "move constructor: ";
	ut::AVLTree<int, ut::String> tree_move(ut::Move(tree));
	ut::Optional<ut::String&> move0_find_result = tree_move.Find(128);
	if (move0_find_result)
	{
		ut::String& str = move0_find_result.Get();
		if (str == "__128")
		{
			if (tree.Find(128))
			{
				report += ". Failed, value was copied, but not moved.";
			}
			else
			{
				report += ". Success";
			}
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}
#endif // CPP_STANDARD >= 2011

	// move assignment test
#if CPP_STANDARD >= 2011
	report += ut::CRet() + "move assignment: ";
	tree = ut::Move(tree_copy0);
	ut::Optional<ut::String&> move1_find_result = tree.Find(128);
	if (move1_find_result)
	{
		ut::String& str = move1_find_result.Get();
		if (str == "__128")
		{
			if (tree_copy0.Find(128))
			{
				report += ". Failed, value was copied, but not moved.";
			}
			else
			{
				report += ". Success";
			}
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}
#endif // CPP_STANDARD >= 2011
}

//----------------------------------------------------------------------------//

SharedPtrTask::SharedPtrTask() : TestTask("Shared pointer")
{ }

void SharedPtrTask::Execute()
{
	static const ut::thread_safety::Mode safety_mode = ut::thread_safety::on;
	ut::WeakPtr<int, safety_mode> weak;

	if (weak.IsValid()) // can't be valid here
	{
		report += "Fail(00)";
		failed_test_counter.Increment();
		return;
	}

	if(true)
	{
		ut::SharedPtr<int, safety_mode> ptr(new int(-1));
		ut::SharedPtr<int, safety_mode> alt_ptr(new int(2));

		ut::SharedPtr<int, safety_mode> ptr0(ptr);

		ptr = alt_ptr;

		ut::WeakPtr<int, safety_mode> weak0(ptr);

		weak0 = alt_ptr;

		weak = weak0;

		if (!weak.IsValid()) // must be valid here
		{
			report += "Fail(01)";
			failed_test_counter.Increment();
		}

		ut::SharedPtr<int, safety_mode> ptr_convertion = weak.Pin();
	}

	if (weak.IsValid()) // can't be valid here
	{
		report += "Fail(02)";
		failed_test_counter.Increment();
		return;
	}

	report += "reached the end. Ok!";
}

//----------------------------------------------------------------------------//
typedef ut::Container<int, char&, ut::UniquePtr<int>, ut::SharedPtr<ut::uint>, ut::Array<ut::byte> > TestContainer;

ContainerTask::ContainerTask() : TestTask("Container template")
{ }

void ContainerTask::Execute()
{
	char b = 3;
	ut::Array<ut::byte> arr;
	arr.Add(0);
	arr.Add(1);
	arr.Add(2);

	ut::UniquePtr<int> uniq_ptr(new int(10));
	ut::SharedPtr<ut::uint> sh_ptr(new ut::uint(12));

	TestContainer c(0, b, Move(uniq_ptr), sh_ptr, Move(arr));

	TestContainer::Item<2>::Type test_unique_ptr(new int(1));

	const int n = TestContainer::size;
	if (n != 5)
	{
		report += "Fail(count)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<0>() != 0)
	{
		report += "Fail(0)";
		failed_test_counter.Increment();
		return;
	}

	c.Get<1>() = 11;
	if (b != 11)
	{
		report += "Fail(1)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<2>().GetRef() != 10)
	{
		report += "Fail(2)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<3>().GetRef() != 12)
	{
		report += "Fail(3)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<4>().GetNum() != 3 || c.Get<4>()[0] != 0 || c.Get<4>()[1] != 1 || c.Get<4>()[2] != 2)
	{
		report += "Fail(4)";
		failed_test_counter.Increment();
		return;
	}

	// compility check
	c.Get<int>() = 2;
	c.Get<char&>() = b;
	c.Get< ut::Array<ut::byte> >() = ut::Array<ut::byte>();
	char& c_test = c.Get<char&>();
	c_test = c.Get<1>();
	const TestContainer& rc = c;
	int i_test = rc.Get<0>();
	const char& cc_test = rc.Get<1>();
	const int& ri_test = rc.Get<int>();

	report += "success";
}

//----------------------------------------------------------------------------//
OptionalTask::OptionalTask() : TestTask("ut::Optional")
{ }

void OptionalTask::Execute()
{
	ut::String test_str("test");

	ut::Optional<ut::String&> opt_str(test_str);

	if (opt_str.Get() != test_str)
	{
		report += "fail! references don't match.";
		failed_test_counter.Increment();
		return;
	}

	// move constructor
	ut::String test_str1(opt_str.Move());
	if (test_str1 != "test")
	{
		report += "fail! while copying to string.";
		failed_test_counter.Increment();
		return;
	}
	else
	{
#if CPP_STANDARD >= 2011
		if (test_str.GetNum() > 0)
		{
			report += "fail! string was copied instead of being moved.";
			failed_test_counter.Increment();
			return;
		}
#endif
	}

	// move assignment
	test_str = "modified_str";
	ut::Optional<ut::String&> opt_str1(test_str1);
	opt_str = ut::Move(opt_str1);
	if (opt_str.Get() != "test")
	{
		report += "fail! ut::optional wasn't moved properly.";
		failed_test_counter.Increment();
		return;
	}

	// assignment
	opt_str = ut::Optional<ut::String&>(test_str);
	if (opt_str.Get() != "modified_str")
	{
		report += "fail! ut::optional must rebind reference on assignment.";
		failed_test_counter.Increment();
		return;
	}

	// const
	const ut::Optional<const ut::String&> opt_str_const(test_str1);
	const ut::String& const_test = opt_str_const.Get();

	report += "success";
}

//----------------------------------------------------------------------------//
ResultTask::ResultTask() : TestTask("ut::Result")
{ }

void ResultTask::Execute()
{
	ut::String test_str = "test";
	const ut::String& test_const_str = "const";

	ut::Result<ut::String&, int> test_result(test_str);
	if (test_result.GetResult() != test_str)
	{
		report += "fail! references don't match.";
		failed_test_counter.Increment();
		return;
	}

	ut::String test_move_str;
	test_move_str = test_result.MoveResult();
#if CPP_STANDARD >= 2011
	if (test_str.GetNum() != 0)
	{
		report += "fail! string was copied instead of being moved.";
		failed_test_counter.Increment();
		return;
	}
#endif

	ut::Result<const ut::String&, int> const_result(test_const_str);
	ut::String test_copy_str = const_result.GetResult();

	ut::Result<const ut::String&, int> const_result_alt(ut::MakeAlt<int>(10));

	report += "success";
}

//----------------------------------------------------------------------------//
PairTask::PairTask() : TestTask("ut::Pair")
{ }

void PairTask::Execute()
{
	ut::String test_str_0 = "test";
	ut::String test_str_1 = "test";
	const ut::String& test_const_str = "const";

	ut::Pair<ut::String&, ut::String> pair_0(test_str_0, "pair");
	ut::Pair<ut::String&, ut::String> pair_1(Move(pair_0));

#if CPP_STANDARD >= 2011
	if (pair_0.second.GetNum() != 0)
	{
		report += "fail! string was copied instead of being moved.";
		failed_test_counter.Increment();
		return;
	}
#endif

	ut::Pair<int, const ut::String&> const_pair(24, test_const_str);
	
	ut::String test_str = "test";
	ut::Pair<ut::String&, const ut::String&> ref_pair(test_str, test_const_str);

	report += "success";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//