#include <string>
#include <libtcod.hpp>

#include "Job.hpp"
#include "Announce.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "GCamp.hpp"

Task::Task(Action act, Coordinate tar, boost::weak_ptr<Entity> ent, ItemCategory itt) :
	target(tar),
	entity(ent),
	action(act),
	item(itt)
{
}

Job::Job(std::string value, JobPriority pri, int z, bool m) :
	_priority(pri),
	completion(ONGOING),
	parent(boost::weak_ptr<Job>()),
	npcUid(-1),
	_zone(z),
	menial(m),
	paused(false),
	waitingForRemoval(false),
	reservedSpot(std::pair<boost::weak_ptr<Stockpile>, Coordinate>(boost::weak_ptr<Stockpile>(), Coordinate(0,0))),
	attempts(0),
	attemptMax(5),
	name(value),
	internal(false)
{
}

Job::~Job() {
	preReqs.clear();
	UnreserveItems();
	UnreserveSpot();
	if (connectedEntity.lock()) connectedEntity.lock()->CancelJob();
	if (reservedSpace.lock()) {
	    reservedSpace.lock()->ReserveSpace(false);
	    reservedSpace = boost::weak_ptr<Container>();
	}
}

void Job::priority(JobPriority value) { _priority = value; }
JobPriority Job::priority() { return _priority; }

bool Job::Completed() {return (completion == SUCCESS || completion == FAILURE);}
void Job::Complete() {completion = SUCCESS;}
std::list<boost::weak_ptr<Job> >* Job::PreReqs() {return &preReqs;}
boost::weak_ptr<Job> Job::Parent() {return parent;}
void Job::Parent(boost::weak_ptr<Job> value) {parent = value;}
void Job::Assign(int uid) {npcUid = uid;}
int Job::Assigned() {return npcUid;}
void Job::zone(int value) {_zone = value;}
int Job::zone() {return _zone;}
bool Job::Menial() {return menial;}
bool Job::Paused() {return paused;}
void Job::Paused(bool value) {paused = value;}
void Job::Remove() {waitingForRemoval = true;}
bool Job::Removable() {return waitingForRemoval;}
int Job::Attempts() {return attempts;}
void Job::Attempts(int value) {attemptMax = value;}
bool Job::Attempt() {
	if (++attempts > attemptMax) return false;
	return true;
}

bool Job::PreReqsCompleted() {
	for (std::list<boost::weak_ptr<Job> >::iterator preReqIter = preReqs.begin(); preReqIter != preReqs.end(); ++preReqIter) {
		if (preReqIter->lock() && !preReqIter->lock()->Completed()) return false;
	}
	return true;
}

bool Job::ParentCompleted() {
	if (!parent.lock()) return true;
	return parent.lock()->Completed();
}


boost::shared_ptr<Job> Job::MoveJob(Coordinate tar) {
	boost::shared_ptr<Job> moveJob(new Job("Move"));
	moveJob->tasks.push_back(Task(MOVE, tar));
	return moveJob;
}

boost::shared_ptr<Job> Job::BuildJob(boost::weak_ptr<Construction> construct) {
	boost::shared_ptr<Job> buildJob(new Job("Build"));
	buildJob->tasks.push_back(Task(MOVEADJACENT, Coordinate(construct.lock()->x(),construct.lock()->y()), construct));
	buildJob->tasks.push_back(Task(BUILD, Coordinate(construct.lock()->x(),construct.lock()->y()), construct));
	return buildJob;
}

void Job::ReserveItem(boost::weak_ptr<Item> item) {
	if (item.lock()) {
        reservedItems.push_back(item);
        item.lock()->Reserve(true);
	}
}

void Job::UnreserveItems() {
	for (std::list<boost::weak_ptr<Item> >::iterator itemI = reservedItems.begin(); itemI != reservedItems.end(); ++itemI) {
		if (itemI->lock()) itemI->lock()->Reserve(false);
	}
	reservedItems.clear();
}

void Job::ReserveSpot(boost::weak_ptr<Stockpile> sp, Coordinate pos) {
    if (sp.lock()) {
        sp.lock()->ReserveSpot(pos, true);
        reservedSpot = std::pair<boost::weak_ptr<Stockpile>, Coordinate>(sp, pos);
    }
}

void Job::UnreserveSpot() {
    if (reservedSpot.first.lock()) {
        reservedSpot.first.lock()->ReserveSpot(reservedSpot.second, false);
        reservedSpot.first.reset();
    }
}

void Job::Fail() {
    completion = FAILURE;
	if (parent.lock()) parent.lock()->Fail();
    preReqs.clear();
	Remove();
}

std::string Job::ActionToString(Action action) {
    switch (action) {
        case MOVE: return std::string("Move");
        case MOVEADJACENT: return std::string("Move adjacent");
        case BUILD: return std::string("Build");
        case TAKE: return std::string("Pick up");
        case DROP: return std::string("Drop");
        case PUTIN: return std::string("Put in");
        case USE: return std::string("Use");
        default: return std::string("???");
    }
}

void Job::ConnectToEntity(boost::weak_ptr<Entity> ent) {
    connectedEntity = ent;
}

void Job::ReserveSpace(boost::weak_ptr<Container> cont) {
    cont.lock()->ReserveSpace(true);
    reservedSpace = cont;
}

