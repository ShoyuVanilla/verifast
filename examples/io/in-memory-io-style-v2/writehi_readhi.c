﻿/**
 * This example uses I/O style contracts for programs that do not perform I/O but
 * just manipulate memory.
 *
 * The write_hi function has an I/O contract and writes 'h','i' to a (ring)buffer.
 * The read_hi function has an I/O contract and reads two bytes from the buffer,
 * if they are not 'h','i', read_hi crashes.
 * The main function does not have an I/O contract and calls write_hi ; read_hi.
 * We thus prove that this program does not crash.
 * The main function closes the necessary I/O predicates required by read_hi and write_hi.
 *
 * It is probably possible to make this file more compact by making some lemmas more generic.
 */

#include "io.h"
#include <malloc.h>

/*@
lemma void t2_inv_holds_generic(int c, list<int> sigma_A)
requires write_io(?t1, c, ?t2) &*& t1 == time(?invar1, _, _, _, ?rely1, _) &*& rely1 == sigma_id &*& t2 == time(?invar2, _, _, _, _, _) &*& true == invar1(sigma_A);
ensures write_io(t1, c, t2) &*& true==invar2(cons(c, sigma_A));
{
  /*
  Prove: ∃ sigma_AA, sigma_B . Invar1(sigma_AA) && Guarantee_+2(sigma_AA, sigma_B) && Rely2(sigma_B, {c} ++ sigma_A).
  It suffices to prove that         ¬∀sigma_AA, sigma_B . ¬(Invar1(sigma_AA) && Guarantee_+2(sigma_AA, sigma_B) && Rely2(sigma_B, {c} ++ sigma_A)).
  Proof by contradiction: assume     ∀sigma_AA, sigma_B . ¬(Invar1(sigma_AA) && Guarantee_+2(sigma_AA, sigma_B) && Rely2(sigma_B, {c} ++ sigma_A)).
  Therefore, for sigma_AA=sigma_A,sigma_B={c}, it holds that ¬(Invar1(sigma_A) && Guarantee_+2(sigma_A, sigma_B) && Rely2(sigma_B, {c} ++ sigma_A)).
  However, the negation also holds.
  */
  open write_io(t1, c, t2);
  assert t1 == time(invar1, ?guarantee_acum1, ?guarantee_thread1, ?guarantee_stack1, rely1, ?rely_stack1);
  get_forall_pair_list_int();
  fixpoint(list<int>, list<int>, bool) guarantee_acum2 = (write_guarantee_acum_fp)(guarantee_acum1, invar1, c);
  assert invar2 == (invar_fp)(invar1,
      (write_guarantee_plus_fp)(invar1, c)
      , rely1
  );
  
  if (! invar2(cons(c, sigma_A))) {
    forall_t_elim(forall_pair_list_int, (invar_fp_helper)(invar1, (write_guarantee_plus_fp)(invar1, c), rely1, cons(c, sigma_A)), pair(sigma_A, cons(c, sigma_A)));
  }
}
@*/

void write(char c)
//@ requires time(?t1) &*& t1==time(_, _, _, _, ?rely1, _) &*& rely1 == sigma_id &*& write_io(t1, c, ?t2) &*& c >= 0 && c < 127;
//@ ensures time(t2);
{
  //@ open time(t1);
  //@ open iostate_shared(?ghost_list_id);
  //@ open iostate(_, _);
  //@ open buffer(?contents);
  //@ assert global_buffer |-> ?global_buffer_ptr;
  //@ assert global_buffer_ptr->ring_buffer |-> ?ring_buffer;
  
  if (ring_buffer_is_full(global_buffer->ring_buffer)){
    // TODO: what to do? Currently single-threaded, so we cannot wait. Require non-empty buffer in precondition?
    while (true) /*@ invariant true; @*/ {}
  }
  ring_buffer_push(global_buffer->ring_buffer, c);
    
  // Prove invar1(contents)
  //@ assert cooked_ghost_list_member_handle(ghost_list_id, ?k, t1);
  //@ cooked_ghost_list_match(ghost_list_id, k);
  //@ assert cooked_ghost_list(_, _, ?k_time_pairs);
  //@ forall_elim(k_time_pairs, (invar_holds)(contents), pair(k, t1));
  
  // Prove invar2(contents). Uses invar1(contents).
  //@ t2_inv_holds_generic(c, contents);
  
  //@ assert length(k_time_pairs) == 1;
  
  // Prove the ghost list is empty.
  //@ cooked_ghost_list_remove(ghost_list_id, k);
  //@ assert cooked_ghost_list(_, _, ?should_be_empty);
  //@ assert should_be_empty == remove(pair(k,t1), k_time_pairs);
  //@ length_remove(pair(k,t1), k_time_pairs);
  //@ assert length(should_be_empty) == 0;
  //@ switch(should_be_empty){case nil: assert true; case cons(x,xs0): assert false;}
  //@ assert should_be_empty == {};
  
  // Get k_time_pairs_new
  //@ int k_t2 = cooked_ghost_list_add(ghost_list_id, t2);
  //@ assert cooked_ghost_list(_, _, ?k_time_pairs_new);
  //@ assert k_time_pairs_new == append(nil, {(pair(k_t2, t2))});
  
  /*@
  if ( ! forall(k_time_pairs_new, (invar_holds)(cons(c, contents)))){
    pair<int, time> ktime = not_forall(k_time_pairs_new, (invar_holds)(cons(c, contents)));
    assert snd(ktime) != t2;
    assert(false);
  }
  @*/
  
  //@ close buffer(cons(c, contents));
  //@ close iostate(ghost_list_id, cons(c, contents));
  //@ close iostate_shared(ghost_list_id);
  //@ close time(t2);
  //@ open write_io(t1, c, t2);
}

int read();
//@ requires time(?t1) &*& read_io(t1, ?c, ?t2);
//@ ensures time(t2) &*& result == c;

bool buffer_create();
//@ requires pointer(&global_buffer, _);
//@ ensures result == true ? buffer({}) : pointer(&global_buffer, ?buffer) ; // buffer contains global pointer.

bool buffer_dispose();
//@ requires buffer(_);
//@ ensures pointer(&global_buffer, _);


void write_hi()
//@ requires time(?t1) &*& t1==time(_, _, _, _, ?rely1, _) &*& rely1 == sigma_id &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3);
//@ ensures time(t3);
{
  //@ open write_io(t1, _, _); // to get rely2 == sigma_id
  write('h');
  write('i');
}

void read_must_be(int c_must_be)
//@ requires time(?t1) &*& read_io(t1, c_must_be, ?t2);
//@ ensures time(t2);
{
  int c_actual = read();
  
  if (c_actual != c_must_be){
    int *nullptr = 0;
    *nullptr = -1;
  }
}


void read_hi()
//@ requires time(?t1) &*& read_io(t1, 'h', ?t2) &*& read_io(t2, 'i', ?t3);
//@ ensures time(t3);
{
  read_must_be('h');
  read_must_be('i');
}

/*@
lemma void t2_inv(list<int> sigma)
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& t2 == time(?invar2, _, _, _, _, _)
  &*& true==invar2(sigma);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& sigma == {'h'};
{
  open write_io(t1, 'h', t2);
  assert t1 == time(?invar1, ?guarantee_acum1, ?guarantee_thread1, ?guarantee_stack1, ?rely1, ?rely_stack1);
  get_forall_pair_list_int();
  fixpoint(list<int>, list<int>, bool) guarantee_acum2 = (write_guarantee_acum_fp)(guarantee_acum1, invar1, 'h');
  assert invar2 == (invar_fp)(invar1,
      (write_guarantee_plus_fp)(invar1, 'h')
      , rely1);
  pair<list<int>, list<int> > sigmapair = not_forall_t(forall_pair_list_int, (invar_fp_helper)(invar1, (write_guarantee_plus_fp)(invar1, 'h'), rely1, sigma));
  assert fst(sigmapair) == {};
}


lemma void t3_inv(list<int> sigma)
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& t3 == time(?invar3, _, _, _, _, _)
  &*& true==invar3(sigma);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& sigma == {'i', 'h'};
{
  open write_io(t1, 'h', t2);
  open write_io(t2, 'i', t3);
  assert t2 == time(?invar2, ?guarantee_acum2, ?guarantee_thread2, ?guarantee_stack2, ?rely2, ?rely_stack2);
  get_forall_pair_list_int();
  fixpoint(list<int>, list<int>, bool) guarantee_acum3 = (write_guarantee_acum_fp)(guarantee_acum2, invar2, 'i');
  assert invar3 == (invar_fp)(invar2,
      (write_guarantee_plus_fp)(invar2, 'i')
      , rely2);
  pair<list<int>, list<int> > sigmapair = not_forall_t(forall_pair_list_int, (invar_fp_helper)(invar2, (write_guarantee_plus_fp)(invar2, 'i'), rely2, sigma));
  t2_inv(fst(sigmapair));
  assert fst(sigmapair) == {'h'};
}


lemma void t4_inv(list<int> sigma)
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& read_io(t3, 'h', ?t4) &*& t4 == time(?invar4, _, _, _, _, _)
  &*& true==invar4(sigma);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& read_io(t3, 'h', t4) &*& sigma == {'i'};
{
  open write_io(t1, 'h', t2);
  open write_io(t2, 'i', t3);
  open read_io(t3, 'h', t4);
  assert t3 == time(?invar3, ?guarantee_acum3, ?guarantee_thread3, ?guarantee_stack3, ?rely3, ?rely_stack3);
  get_forall_pair_list_int();
  fixpoint(list<int>, list<int>, bool) guarantee_acum4 = (read_guarantee_acum_fp)(guarantee_acum3, invar3, 'h');
  assert invar4 == (invar_fp)(invar3,
      (read_guarantee_plus_fp)(invar3, 'h')
      , rely3);
  pair<list<int>, list<int> > sigmapair = not_forall_t(forall_pair_list_int, (invar_fp_helper)(invar3, (read_guarantee_plus_fp)(invar3, 'h'), rely3, sigma));
  t3_inv(fst(sigmapair));
  assert fst(sigmapair) == {'i', 'h'};
}


lemma void t2_inv_holds()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& t2 == time(?invar2, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& true==invar2({'h'});
{
  t2_inv_holds_generic('h', {});
}

lemma void t3_inv_holds()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& t3 == time(?invar3, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& true==invar3({'i', 'h'});
{
  t2_inv_holds();
  open write_io(t1, 'h', t2);
  t2_inv_holds_generic('i', {'h'});
}


lemma void t4_inv_holds()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& read_io(t3, 'h', ?t4) &*& t4 == time(?invar4, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& read_io(t3, 'h', t4) &*& true==invar4({'i'});
{
  t3_inv_holds();
  open read_io(t3, 'h', t4);
  assert t3 == time(?invar3, ?guarantee_acum3, ?guarantee_thread3, ?guarantee_stack3, ?rely3, ?rely_stack3);
  get_forall_pair_list_int();
  fixpoint(list<int>, list<int>, bool) guarantee_acum4 = (read_guarantee_acum_fp)(guarantee_acum3, invar3, 'h');
  assert invar4 == (invar_fp)(invar3,
      (read_guarantee_plus_fp)(invar3, 'h')
      , rely3
  );
  
  if (! invar4({'i'})) {
    forall_t_elim(forall_pair_list_int, (invar_fp_helper)(invar3, (read_guarantee_plus_fp)(invar3, 'h'), rely3, {'i'}), pair({'i', 'h'}, {'i'}));
    
    open write_io(t1, 'h', t2); open write_io(t2, 'i', t3); /* to get */ assert rely3 == sigma_id;
    
    assert true == invar_fp_helper(invar3, (read_guarantee_plus_fp)(invar3, 'h'), rely3, {'i'}, pair({'i', 'h'}, {'i'}));
    // Rewrite:
    assert true == ! (
      invar3({'i','h'})
      && read_guarantee_plus_fp(invar3, 'h', {'i','h'}, {'i'})
      && rely3({'i'}, {'i'})
    );
    // Therefore, because these two are true...
    assert true==invar3({'i','h'});
    assert true==rely3({'i'}, {'i'});
    // This one must be false:
    assert false == read_guarantee_plus_fp(invar3, 'h', {'i','h'}, {'i'});
    // Rewrite:
    assert false == (invar3({'i','h'}) && {'i','h'} != nil && {'i'} == remove_last({'i','h'}) && 'h' == last({'i','h'}));
    // To obtain a contradiction, one is false. VeriFast can see these are true:
    assert true == invar3({'i','h'});
    assert 'h' == last({'i','h'});
    // Therefore:
    assert {'i'} != remove_last({'i','h'});
    // While the negation is also true.
    assert false;
  }
}

lemma void invar_of_prophecy_is_true()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& t3 == time(?invar3, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& true == read_prophecy(invar3, 'h');
{
   if ( forall_list_int((read_prophecy_helper)('h', invar3))){
     get_forall_list_int();
     list<int> sigma = {'i','h'};
     forall_t_elim(forall_list_int, (read_prophecy_helper)('h', invar3), sigma);
     assert true!=invar3(sigma);
     t3_inv_holds();
   }
}

lemma void invar_of_prophecy2_is_true()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3)
  &*& read_io(t3, 'h', ?t4) &*& t4 == time(?invar4, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& read_io(t3, 'h', t4) &*& true == read_prophecy(invar4, 'i');
{
  if ( forall_list_int((read_prophecy_helper)('i', invar4))){
     get_forall_list_int();
     list<int> sigma = {'i'};
     forall_t_elim(forall_list_int, (read_prophecy_helper)('i', invar4), sigma);
     assert true!=invar4(sigma);
     t4_inv_holds();
   }
}

lemma void prophecy_implies_c_eq_h()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3) &*& t3 == time(?invar3, _, _, _, _, _)
  &*& read_io(t3, ?c, ?t4);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& read_io(t3, c, t4) &*& c == 'h';
{
  /*
  We want to prove: c == 'h'.
  invar_of_prohecy_is_true proves that there is an x such that the prophecy invariant holds for x,
  therefore the prophecy invariant holds for c.
  The prophecy invariant is: invar(c) <=> ¬∀sigma.¬(I3(sigma) && sigma != nil && last(sigma) == c).
  Because                                 ¬∀sigma.¬(I3(sigma) && sigma != nil && last(sigma) == c),
  there must be a sigma such that                  (I3(sigma) && sigma != nil && last(sigma) == c).
  Take such a sigma. Because I3(sigma), sigma == {'i','h'} (use t3_inv to prove that).
  Therefore, last(sigma) == c and last(sigma) == 'h'.
  Thus, c == 'h'.
  */

  invar_of_prophecy_is_true(); /* shows that: */ assert true==read_prophecy(invar3, 'h');

  open read_io(t3, c, t4);
  assert prophecy_int((read_prophecy)(invar3), c);
  prophecy_invar(c, 'h');
  assert true == read_prophecy(invar3, c);
  assert ! forall_list_int((read_prophecy_helper)(c, invar3));
  list<int> sigma = not_forall_t(forall_list_int, (read_prophecy_helper)(c, invar3));
  assert (invar3(sigma) && sigma != nil && last(sigma) == c);
  t3_inv(sigma);
}

lemma void prophecy_implies_ci_eq_i()
requires time(?t1) &*& t1 == init_time() &*& write_io(t1, 'h', ?t2) &*& write_io(t2, 'i', ?t3)
  &*& read_io(t3, 'h', ?t4) &*& read_io(t4, ?ci, ?t5) &*& t4 == time(?invar4, _, _, _, _, _);
ensures time(t1) &*& write_io(t1, 'h', t2) &*& write_io(t2, 'i', t3) &*& read_io(t3, 'h', t4) &*& read_io(t4, ci, t5) &*& ci == 'i';
{
  invar_of_prophecy2_is_true(); /* shows that: */ assert true==read_prophecy(invar4, 'i');
  open read_io(t4, ci, t5);
  get_forall_list_int();
  assert prophecy_int((read_prophecy)(invar4), ci);
  prophecy_invar(ci, 'i');
  assert true == read_prophecy(invar4, ci);
  assert ! forall_list_int((read_prophecy_helper)(ci, invar4));
  list<int> sigma = not_forall_t(forall_list_int, (read_prophecy_helper)(ci, invar4));
  assert (invar4(sigma) && sigma != nil && last(sigma) == ci);
  t4_inv(sigma);
}

@*/


bool initialize_io()
//@ requires pointer(&global_buffer, _);
/*@ ensures
  result == true ?
    time(?t1) &*& t1 == init_time()
  : // failure:
    pointer(&global_buffer, ?buffer);
@*/
{
  if (!buffer_create()){
    return false;
  }
  //@ time t1 = init_time();
  //@ get_forall_pair_list_int();
  //@ int ghost_list_id = create_cooked_ghost_list<time>();
  //@ cooked_ghost_list_add(ghost_list_id, t1);
  //@ close iostate(ghost_list_id, {});
  //@ close iostate_shared(ghost_list_id);
  //@ close time(t1);
  return true;
}


int main(int argc, char **argv) //@ : main_full(writehi_readhi)
//@ requires module(writehi_readhi, true) &*& [_]argv(argv, argc, ?arguments);
//@ ensures true;
{
  //@ open_module();
  
  // Create time(t1).
  bool ret = initialize_io();
  if (!ret){
    //@ leak pointer(&global_buffer, _);
    return -1;
  }
  //@ assert time(?t1);
  
  // Create write_io chunks.
  //@ get_forall_pair_list_int();
  //@ close write_io(t1, 'h', ?t2); // can get auto-closed, but now we get t2 for free.
  //@ close write_io(t2, 'i', ?t3);
    
  // Create t4.
  //@ assert t3 == time(?invar3, _, _, _, _, _);
  //@ fixpoint(int, bool) invar_of_prophecy = (read_prophecy)(invar3);
  //@ int c = prophecy_create(invar_of_prophecy);
  //@ time t4 = read_io_t2(t3, c);
  
  // Create read_io(t3, c, t4).
  //@ get_forall_list_int();
  //@ close read_io(t3, c, t4);
  
  // Prove c == 'h'.
  //@ prophecy_implies_c_eq_h();
  
  // Create t5.
  //@ assert t4 == time(?invar4, _, _, _, _, _);
  //@ fixpoint(int, bool) invar_of_prophecy2 = (read_prophecy)(invar4);
  //@ int c_i = prophecy_create(invar_of_prophecy2);
  //@ time t5 = read_io_t2(t4, c_i);
  
  // Create read_io(t4, c_i, t5).
  //@ close read_io(t4, c_i, t5);
  
  // Prove ci == 'i'.
  //@ prophecy_implies_ci_eq_i();

  write_hi();
  
  read_hi();
  
  //@ leak time(_);
  return 0;
  
}


