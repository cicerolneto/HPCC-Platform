/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */

// WUXMLInfo.cpp: implementation of the CWUXMLInfo class.
//
//////////////////////////////////////////////////////////////////////
#include "WUXMLInfo.hpp"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include "dasds.hpp"
#include "exception_util.hpp"

#include "stdio.h"
#define SDS_TIMEOUT 3000


CWUXMLInfo::CWUXMLInfo()
{

}

CWUXMLInfo::~CWUXMLInfo()
{

}

void CWUXMLInfo::buildXmlActiveWuidStatus(const char* ClusterName, IEspECLWorkunit& wuStructure)
{
    try{
        Owned<IRemoteConnection> conn = querySDS().connect("/Status",myProcessSession(),RTM_LOCK_READ,SDS_TIMEOUT);
        if (!conn)
            throw MakeStringException(ECLWATCH_CANNOT_CONNECT_DALI, "Could not connect to Dali server.");

        Owned<IPropertyTreeIterator> it = conn->queryRoot()->getElements("Thor");
        CWUXMLInfo wuidInfo;
        ForEach(*it)
        {
            const char* nodeName = it->query().queryProp("@name");
            if (nodeName && strcmp(ClusterName,nodeName) == 0)
            {
                const char *wuid = it->query().queryProp("WorkUnit");
                if(wuid)
                {
                    wuidInfo.buildXmlWuidInfo(wuid,wuStructure);
                }
            }
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlActiveWuidStatus");
    }

}

bool CWUXMLInfo::buildXmlActionList(IConstWorkUnit &wu, StringBuffer& statusStr)
{
    return true;
}

bool CWUXMLInfo::buildXmlWuidInfo(IConstWorkUnit &wu, IEspECLWorkunit& wuStructure,bool bDescription)
{
    try
    {
        SCMStringBuffer buf;
        wuStructure.setProtected((wu.isProtected() ? 1 : 0));
        buf.clear();
        wuStructure.setWuid(wu.getWuid(buf).str()); 
        buf.clear();
        wuStructure.setOwner(wu.getUser(buf).str());
        buf.clear();
        wuStructure.setJobname(wu.getJobName(buf).str());
        buf.clear();
        wuStructure.setCluster(wu.getClusterName(buf).str());
        wuStructure.setStateID(wu.getState());
        buf.clear();
        wuStructure.setState(wu.getStateDesc(buf).str());
        if (bDescription)
        {
            buf.clear();
            wuStructure.setDescription((wu.getDebugValue("description", buf)).str());
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlWuidInfo");
    }
    return true;
}



bool CWUXMLInfo::buildXmlWuidInfo(const char* wuid, IEspECLWorkunit& wuStructure, bool bDescription)
{
    try{
        Owned<IWorkUnitFactory> factory = getWorkUnitFactory();
        Owned<IConstWorkUnit> wu = factory->openWorkUnit(wuid, false);
        if (wu)
        {
            return buildXmlWuidInfo(*wu.get(),wuStructure,bDescription);
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlWuidInfo");
    }
    return false;
}

bool CWUXMLInfo::buildXmlWuidInfo(IConstWorkUnit &wu, StringBuffer& wuStructure,bool bDescription)
{
    try{
      SCMStringBuffer buf;

        wuStructure.append("<Workunit>");
        wuStructure.appendf("<Protected>%d</Protected>",(wu.isProtected() ? 1 : 0));
        wuStructure.appendf("<Wuid>%s</Wuid>",wu.getWuid(buf).str());
      buf.clear();
        wuStructure.appendf("<Owner>%s</Owner>",wu.getUser(buf).str());
      buf.clear();
        wuStructure.appendf("<Jobname>%s</Jobname>",wu.getJobName(buf).str());
      buf.clear();
        wuStructure.appendf("<Cluster>%s</Cluster>",wu.getClusterName(buf).str());
      buf.clear();
        wuStructure.appendf("<State>%s</State>",wu.getStateDesc(buf).str());
      buf.clear();
        if (bDescription)
      {
            wuStructure.appendf("<Description>%s</Description>",
            (wu.getDebugValue("description", buf)).str());
         buf.clear();
      }
        wuStructure.append("</Workunit>");
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlWuidInfo");
    }

    return true;
}

bool CWUXMLInfo::buildXmlGraphList(IConstWorkUnit &wu,IPropertyTree& XMLStructure)
{
    try {
      SCMStringBuffer buf;

        IPropertyTree* resultTree = XMLStructure.addPropTree("WUGraphs", createPTree(true));    
        Owned<IConstWUGraphIterator> graphs = &wu.getGraphs(GraphTypeAny);
        ForEach(*graphs)
        {
            IConstWUGraph &graph = graphs->query();
            if(!graph.isValid())
                continue;
            IPropertyTree * p   =   resultTree->addPropTree("WUGraph", createPTree(true));
            p->setProp("Wuid",      wu.getWuid(buf).str());
         buf.clear();
            p->setProp("Name",      graph.getName(buf).str());
         buf.clear();
         // MORE? (debugging)
            p->setPropInt("Running",    (((wu.getState() == WUStateRunning)||(wu.getState() == WUStateDebugPaused)||(wu.getState() == WUStateDebugRunning)) ? 1 : 0));
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlGraphList");
    }
    return true;
}

bool CWUXMLInfo::buildXmlExceptionList(IConstWorkUnit &wu,IPropertyTree& XMLStructure)
{
    try {
        if (wu.getExceptionCount())
        {
            Owned<IConstWUExceptionIterator> exceptions = &wu.getExceptions();
            ForEach(*exceptions)
            {
                SCMStringBuffer x, y;
                IPropertyTree * p   =   XMLStructure.addPropTree("Exceptions", createPTree(true));
                p->setProp("Source",exceptions->query().getExceptionSource(x).str());
                p->setProp("Message",exceptions->query().getExceptionMessage(y).str());
            }
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlExceptionList");
    }
    return true;
}

bool CWUXMLInfo::buildXmlResultList(IConstWorkUnit &wu,IPropertyTree& XMLStructure)
{
    try{
        IPropertyTree* resultsTree = XMLStructure.addPropTree("WUResults", createPTree(true));  
        Owned<IConstWUResultIterator> results = &wu.getResults();
        ForEach(*results)
        {
            IConstWUResult &r = results->query();
            if (r.getResultSequence() != -1)
            {
                SCMStringBuffer x;
                StringBuffer xStr;
                StringBuffer value,link;
                r.getResultName(x);
                if (!x.length())
                {
                    value.appendf("Result %d",r.getResultSequence()+1);
                }
                if (r.getResultStatus() == ResultStatusUndefined)
                {
                    value.append("   [undefined]");
                }
                else if (r.isResultScalar())
                {
                    SCMStringBuffer x;
                    r.getResultXml(x);
                    try
                    {
                        Owned<IPropertyTree> props = loadPropertyTree(x.str(), true);
                        IPropertyTree *row = props->queryPropTree("Row");
                        IPropertyTree *val = row->queryPropTree("*");
                        value.append(val->queryProp(NULL));
                    }
                    catch(...)
                    {
                        value.append("[value not available]");
                    }
                }
                else
                {
                    value.append("   [");
                    value.append(r.getResultTotalRowCount());
                    value.append(" rows]");
                    link.append(r.getResultSequence());
                }
                IPropertyTree* result = resultsTree->addPropTree("WUResult", createPTree(true));    
                result->setProp("Name",x.str());
                result->setProp("Link",link.str());
                result->setProp("Value",value.str());
                SCMStringBuffer filename;
                r.getResultLogicalName(filename);
                StringBuffer lf;
                if(filename.length())
                {
                    lf.append(filename.str());

                }
                result->setProp("LogicalName",lf.str());
            }
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlExceptionList");
    }
    return true;
}

bool CWUXMLInfo::buildXmlTimimgList(IConstWorkUnit &wu,IPropertyTree& XMLStructure)
{
    try{
        IPropertyTree* timingTree = XMLStructure.addPropTree("Timings", createPTree(true)); 
        Owned<IStringIterator> times = &wu.getTimers();
        ForEach(*times)
        {
            SCMStringBuffer name;
            times->str(name);
            SCMStringBuffer value;
            unsigned count = wu.getTimerCount(name.str(), NULL);
            unsigned duration = wu.getTimerDuration(name.str(), NULL);
            StringBuffer fd;
            formatDuration(fd, duration);
            for (unsigned i = 0; i < name.length(); i++)
                if (name.s.charAt(i)=='_') 
                    name.s.setCharAt(i, ' ');

            IPropertyTree* Timer = timingTree->addPropTree("Timer", createPTree(true)); 
            Timer->setProp("Name",name.str());
            Timer->setProp("Value",fd.str());
            Timer->setPropInt("Count",count);
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlTimimgList");
    }

    return true;
}

bool CWUXMLInfo::buildXmlLogList(IConstWorkUnit &wu,IPropertyTree& XMLStructure)
{
    try{
        IPropertyTree* logTree = XMLStructure.addPropTree("WULog", createPTree(true));  
        Owned <IConstWUQuery> query = wu.getQuery();
        if(query)
        {
            SCMStringBuffer qname;
            query->getQueryCppName(qname);
            if(qname.length())
            {
                logTree->setProp("Cpp",qname.str());
            }
            qname.clear();
            query->getQueryResTxtName(qname);
            if(qname.length())
            {
                logTree->setProp("ResTxt",qname.str());
            }
            qname.clear();
            query->getQueryDllName(qname);
            if(qname.length())
            {
                logTree->setProp("Dll",qname.str());
            }
            qname.clear();
            wu.getDebugValue("ThorLog", qname);
            if(qname.length())
            {
                logTree->setProp("ThorLog",qname.str());
            }
        }
    }
    catch(IException* e){   
      StringBuffer msg;
      e->errorMessage(msg);
        WARNLOG(msg.str());
        e->Release();
    }
    catch(...){
        WARNLOG("Unknown Exception caught within CWUXMLInfo::buildXmlLogList");
    }
    return true;
}
void CWUXMLInfo::formatDuration(StringBuffer &ret, unsigned ms)
{
    unsigned hours = ms / (60000*60);
    ms -= hours*(60000*60);
    unsigned mins = ms / 60000;
    ms -= mins*60000;
    unsigned secs = ms / 1000;
    ms -= secs*1000;
    bool started = false;
    if (hours > 24)
    {
        ret.appendf("%d days ", hours / 24);
        hours = hours % 24;
        started = true;
    }
    if (hours || started)
    {
        ret.appendf("%d:", hours);
        started = true;
    }
    if (mins || started)
    {
        ret.appendf("%d:", mins);
        started = true;
    }
    if (started)
        ret.appendf("%02d.%03d", secs, ms);
    else
        ret.appendf("%d.%03d", secs, ms);
}
