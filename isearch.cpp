#include "gl_const.h"
#include "isearch.h"


ISearch::ISearch()
{
    hweight = 1;
    breakingties = CN_SP_BT_GMAX;
    sizelimit = CN_SP_SL_NOLIMIT;
    open=NULL;
    openSize=0;
}

ISearch::~ISearch(void)
{
    if(open)
    {
        delete []open;
    }
}

double ISearch::MoveCost(int start_i, int start_j, int fin_i, int fin_j, const EnvironmentOptions &options)
{
    if((start_i-fin_i) != 0 && (start_j-fin_j) != 0)
        return options.diagonalcost;
    return options.linecost;
}

bool ISearch::stopCriterion()
{
    if(openSize==0)
    {
        std::cout << "OPEN list is empty!" << std::endl;
        return true;
    }
    return false;
}

SearchResult ISearch::startSearch(ILogger *Logger, const Map &map, const EnvironmentOptions &options)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    open=new NodeList[map.height];
    Node curNode;
    curNode.i = map.start_i;
    curNode.j = map.start_j;
    curNode.g = 0;
    curNode.H = computeHFromCellToCell(curNode.i,curNode.j, map.goal_i, map.goal_j,options);
    curNode.F = hweight * curNode.H;
    curNode.parent = 0;

    addOpen(curNode);
    int closeSize=0;
    bool pathfound=false;
    while(!stopCriterion())
    {
        curNode = findMin(map.height);
        close.insert({curNode.i*map.width+curNode.j,curNode});
        closeSize++;
        open[curNode.i].List.pop_front();
        openSize--;
        if(curNode.i==map.goal_i && curNode.j==map.goal_j)
        {
            pathfound=true;
            break;
        }
        std::list<Node> successors=findSuccessors(curNode,map,options);
        std::list<Node>::iterator it=successors.begin();
        while(it!=successors.end())
        {
            it->parent = &(close.find(curNode.i*map.width+curNode.j)->second);
            it->H = computeHFromCellToCell(it->i,it->j,map.goal_i,map.goal_j,options);
            *it=resetParent(*it, *it->parent, map, options);
            it->F = it->g + hweight * it->H;
            addOpen(*it);
            it++;
        }
        Logger->writeToLogOpenClose(open,close,map.height);

    }
    //Поиск завершился!
    sresult.pathfound = false;
    sresult.nodescreated = closeSize + openSize;
    sresult.numberofsteps = closeSize;
    if(pathfound)
    {
        sresult.pathfound = true;
        //путь восстанолвенный по обратным указателям (для всех алгоритмов)
        makePrimaryPath(curNode);
        sresult.hppath = &hppath;
        sresult.pathlength = curNode.g;
    }
    //Т.к. восстановление пути по обратным указателям - неотъемлемая часть алгоритмов, время останавливаем только сейчас!
    end = std::chrono::system_clock::now();
    sresult.time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count())/1000000;
    //перестроенный путь (hplevel, либо lplevel, в зависимости от алгоритма)
    if(pathfound)
        makeSecondaryPath(map, curNode);
    return sresult;
}

Node ISearch::findMin(int size)
{
    Node min;
    min.F=std::numeric_limits<double>::infinity();
    for(int i=0; i<size; i++)
    {
        if(!open[i].List.empty())
            if(open[i].List.begin()->F<=min.F)
            {
                if (open[i].List.begin()->F == min.F)
                {
                    switch(breakingties)
                    {
                        case CN_SP_BT_GMAX:
                        {
                            if (open[i].List.begin()->g >= min.g)
                            {
                                min=*open[i].List.begin();
                            }
                        }
                        case CN_SP_BT_GMIN:
                        {
                            if (open[i].List.begin()->g <= min.g)
                            {
                                min=*open[i].List.begin();
                            }
                        }
                    }
                }
                else
                min=*open[i].List.begin();
            }
    }
    return min;

}


std::list<Node> ISearch::findSuccessors(Node curNode, const Map &map, const EnvironmentOptions &options)
{
    Node newNode;
    std::list<Node> successors;
    for(int i = -1; i <= +1; i++)
    {
        for(int j = -1; j <= +1; j++)
        {
            if((i != 0 || j != 0) && map.CellOnGrid(curNode.i+i,curNode.j+j) && (map.CellIsTraversable(curNode.i+i,curNode.j+j)))
            {
                if(options.allowdiagonal == CN_SP_AD_FALSE)
                    if(i != 0 && j != 0)
                        continue;
                if(options.allowsqueeze == CN_SP_AS_FALSE)
                {
                    if(i != 0 && j != 0)
                        if(map.CellIsObstacle(curNode.i,curNode.j+j) && map.CellIsObstacle(curNode.i+i,curNode.j))
                            continue;
                }
                if(close.find((curNode.i+i)*map.width+curNode.j+j)==close.end())
                {
                    newNode.i = curNode.i+i;
                    newNode.j = curNode.j+j;
                    newNode.g = curNode.g + MoveCost(curNode.i,curNode.j,curNode.i+i,curNode.j+j,options);
                    successors.push_front(newNode);
                }
            }
        }
    }
    return successors;
}

void ISearch::makePrimaryPath(Node curNode)
{
    Node current=curNode;
    while(current.parent)
    {
        lppath.List.push_front(current);
        current=*current.parent;
    }
    lppath.List.push_front(current);
    sresult.lppath = &lppath; //здесь у sresult - указатель на константу.
}

void ISearch::makeSecondaryPath(const Map &map, Node curNode)
{
    std::list<Node>::const_iterator iter=lppath.List.begin();
    int curI, curJ, nextI, nextJ, moveI, moveJ;
    hppath.List.push_back(*iter);

    while(iter != --lppath.List.end())
    {
        curI = iter->i;
        curJ = iter->j;
        iter++;
        nextI = iter->i;
        nextJ = iter->j;
        moveI = nextI-curI;
        moveJ = nextJ-curJ;
        iter++;
        if((iter->i - nextI) != moveI || (iter->j - nextJ) != moveJ)
            hppath.List.push_back(*(--iter));
        else
        iter--;
    }
    sresult.hppath = &hppath;
}
