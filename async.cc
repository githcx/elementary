#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <queue>
using namespace std;

struct Job
{
		void (*worker)(void* arg);
		void* arg;
};

class AsyncJobPool
{
	public:
		AsyncJobPool(int concur_ = 5):concur(concur_),cont(true){}
		int concur;
		bool cont;
		mutex m;
		condition_variable c;
		queue<Job> jobs;
		vector<shared_ptr<thread>> vpth;

		void PushJob(Job j)
		{
			unique_lock<mutex> lck(m);
			if( !cont ) return;
			jobs.push(j);
			c.notify_one();
		}

		shared_ptr<Job> PopJob()
		{
			unique_lock<mutex> lck(m);
			while(jobs.empty())
			{
				if( !cont ) return NULL;
				c.wait(lck);
			}

			Job ret = jobs.front(); jobs.pop();
			return make_shared<Job>(ret);
		}

		void Start() {
			for(int i = 0; i < concur; i++)
			vpth.push_back( make_shared<thread>(
				[&]{
						while(true)
						{
							auto j = PopJob();
							if( j == NULL )
								break;

							j->worker(j->arg);
						}

						if( !cont )
						{
							c.notify_all();
						}
					}
			) );
		}

		void Stop()
		{
			cont = false;
			c.notify_all();
			for(auto& pth : vpth)
				pth->join();
		}
};

struct NotifiedArg
{
		condition_variable* c;
		mutex* m;
		atomic<int>* cnt;
		Job j;
};

void NotifiedWorker(void* nArg)
{
	shared_ptr<NotifiedArg> p_nArg((NotifiedArg*)nArg);

	p_nArg->j.worker(p_nArg->j.arg);  // do real job

	unique_lock<mutex> lck(*(p_nArg->m));
	--*(p_nArg->cnt);
	p_nArg->c->notify_one();
}

class AsyncJobGroup
{
	public:
		AsyncJobGroup(AsyncJobPool& pool_):pool(pool_){ cnt = 0; }
		AsyncJobPool& pool;
		condition_variable c;
		mutex m;
		atomic<int> cnt;

		void CommitJob(Job j)
		{
			cnt++;
			auto notifiedArg = new NotifiedArg;
			notifiedArg->c = &c;
			notifiedArg->m = &m;
			notifiedArg->cnt = &cnt;
			notifiedArg->j = j;
			Job pj = {&NotifiedWorker, notifiedArg};
			pool.PushJob(pj);
		}

		void Wait()
		{
				unique_lock<mutex> lck(m);
				while(cnt)
				{
					c.wait(lck);
				}

				return;
		}
};

void DecreaseCounter(void *arg)
{
		atomic<int>& n = *(atomic<int>*)arg;

		static mutex g_mtx;
		unique_lock<mutex> lck(g_mtx);
		cout << "Task id = " << n << " finished" << endl;

		n--;
}

int main()
{
	atomic<int> n;
	n = 5;
	int data[20];
	Job jarr[20];
	for(int i = 0; i < 20; i++)
	{
		data[i] = i + 1;
		jarr[i] = {&DecreaseCounter, &data[i]};
	}

	AsyncJobPool pool;
	
	pool.Start();

	thread t1(
	[&]
	{
		AsyncJobGroup group(pool);
		for(int i = 0; i < 10; i++)
			group.CommitJob(jarr[i]);
		group.Wait();
	}
	);

	thread t2(
	[&]
	{
		AsyncJobGroup group(pool);
		for(int i = 0; i < 10; i++)
			group.CommitJob(jarr[i+10]);
		group.Wait();
	}
	);

	t1.join();
	t2.join();

	pool.Stop();

	return 0;
}
