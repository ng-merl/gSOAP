/*	wsdlcpp.java

The contents of this file are subject to the MPL 1.1 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/wsdllicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developers of the Original Code are Kiran Kaja and Robert A. van Engelen.
Copyright (C) 2000-2002 Kiran Kaja and Robert A. van Engelen. All Rights Reserved.

*/

import java.io.IOException;
import java.util.*;
import java.io.*;

import org.xml.sax.SAXException;

import org.w3c.dom.Document;
import org.w3c.dom.DocumentType;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

// Import your vendor's DOM parser
import org.apache.xerces.parsers.DOMParser;

public class wsdlcpp
{
    boolean outc = false;

    Hashtable tobeProcessedArrays = new Hashtable();
    
    Hashtable processedArrays = new Hashtable();

    Vector processedEnums = new Vector();

    Vector processedTypeDefs = new Vector();

    Vector allArrays = new Vector();
    
    Hashtable structureNodes = new Hashtable();

    Hashtable elementNodes = new Hashtable();

    Hashtable bindingsList = new Hashtable();

    Hashtable soapActions = new Hashtable();

    
    //    Hashtable soapHeaders = new Hashtable();
    Vector soapHeaders = new Vector();
    Vector printedClasses = new Vector();
    
    Hashtable nameSpaces = new Hashtable();
   /*sets the name spaces of the entire documents it looks only into the
     * definitions node*/
    String docNS=null;
    String definitionNS=null;
    String soapNS=null;
    String defaultNS=null;
    String headerNS=null;
    
    Hashtable allPrimitiveXSDDataType = new Hashtable();
    Hashtable allPrimitiveXSDDataTypeEquivalent = new Hashtable();

    Hashtable allPrimitiveSOAPDataType = new Hashtable();
    Hashtable allPrimitiveSOAPDataTypeEquivalent = new Hashtable();

    boolean defWsdl=false;
    boolean defSoap=false;

    public void setNameSpaces(Node node)//get the doucument node
    {
	String wsdlNS=null;
	String xsdNS=null;
	String encodingNS=null;
	
	NodeSearch defSearch = new NodeSearch(node,"definitions",new Hashtable());

	Node defNode = defSearch.getNextPartialMatch();

	String tns=null;
	String defans=null;

	if(defNode==null)
	    {
		System.out.println("NO Definitions node found");
		return;
	    }

	String defname = defNode.getNodeName();

	docNS = defname.substring(0,defname.indexOf("definitions"));


	NamedNodeMap attributes = defNode.getAttributes();

	for (int i=0; i<attributes.getLength(); i++)
	    {
		Node current = attributes.item(i);

		if(current.getNodeName().compareToIgnoreCase("targetNamespace")!=0)
		    {
			nameSpaces.put(current.getNodeValue(),current.getNodeName());

			if(current.getNodeName().startsWith("xmlns:")&&(globalNameSpaces.get(current.getNodeValue())==null))
			    globalNameSpaces.put(current.getNodeValue(),current.getNodeName());
		    }
		else
		    {
			tns =  current.getNodeValue();
			continue;
		    }

		if(current.getNodeName().compareToIgnoreCase("xmlns")==0)
		    {
			defans =  current.getNodeValue();
			if(defans.endsWith("/wsdl/"))
			    {
				defWsdl = true;
			    }
			if(defans.endsWith("/soap/"))
			    {
				defSoap = true;
			    }
			continue;
		    }
		
		if(current.getNodeValue().endsWith("/wsdl/"))
		    {
			StringTokenizer st = new StringTokenizer(current.getNodeName(),":");
			if(st.countTokens()==2)
			    {
				st.nextToken();
				wsdlNS =new String(st.nextToken()+":");
			    }
		    }

		if(current.getNodeValue().endsWith("/echoheader/"))
		    {
			StringTokenizer st = new StringTokenizer(current.getNodeName(),":");
			if(st.countTokens()==2)
			    {
				st.nextToken();
				headerNS =new String(st.nextToken()+":");
			    }
		    }
		
		if(current.getNodeValue().endsWith("/soap/"))
		    {
			StringTokenizer st = new
			    StringTokenizer(current.getNodeName(),":");
			if(st.countTokens()==2)
			    {
				st.nextToken();
				soapNS =new String(st.nextToken()+":");
			    }
			else
			    {
				//System.out.println("Found soap name space but name has more than 2 elements on tokenizer :");
				
			    }
			  
		    }
		
		if(current.getNodeValue().endsWith("/soap/encoding/"))
		    {
			StringTokenizer st = new
			    StringTokenizer(current.getNodeName(),":");
			if(st.countTokens()==2)
			    {
				st.nextToken();
				encodingNS =new String(st.nextToken()+":");
			    }
			else
			    {
				//System.out.println("Found encoding name space but name has more than 2 elements on tokenizer :");
				
			    }
		    }
		
		if(current.getNodeValue().endsWith("/XMLSchema"))
		    {
			StringTokenizer st = new
			    StringTokenizer(current.getNodeName(),":");
			if(st.countTokens()==2)
			    {
				st.nextToken();
				xsdNS =new String(st.nextToken()+":");
			    }
			else
			    {
				//System.out.println("Found xsd name space but name has more than 2 elements on tokenizer :");
				
			    }
		    }

	    }

	
	if(tns!=null)
	    {
		for (int i=0; i<attributes.getLength(); i++)
		    {
			Node current = attributes.item(i);
			if(current.getNodeName().compareToIgnoreCase("targetNamespace")==0)
			    continue;

			if(current.getNodeName().compareToIgnoreCase("xmlns")==0)
			    continue;
			
			if(current.getNodeValue().equals(tns))
			    {
				
				StringTokenizer st = new
				    StringTokenizer(current.getNodeName(),":");
				if(st.countTokens()==2)
				    {
					
					Hashtable nsHt = new Hashtable();
					
					nsHt.put(current.getNodeName(),current.getNodeValue());
					
					definitionNS = getNS(getName(current.getNodeName())+":",nsHt);

					/*st.nextToken();
					definitionNS =new String(st.nextToken()+":");*/
				    }
				else
				    {
					//System.out.println("Found default name space but name has more than 2 elements on tokenizer :");
				    }
			    }
			
		    }
	    }
	else
	    {
		System.out.println("TargetName space not found not found");
	    }
	 

	if(defans!=null)
	    {
		for (int i=0; i<attributes.getLength(); i++)
		    {
			Node current = attributes.item(i);
			if(current.getNodeName().compareToIgnoreCase("targetNamespace")==0)
			    continue;
			if(current.getNodeName().compareToIgnoreCase("xmlns")==0)
			    continue;
			
			if(current.getNodeValue().equals(defans))
			    {
				StringTokenizer st = new
				    StringTokenizer(current.getNodeName(),":");
				if(st.countTokens()==2)
				    {
					st.nextToken();
					defaultNS =new String(st.nextToken()+":");
					
				    }
				else
				    {
					//System.out.println("Found default name space but name has more than 2 elements on tokenizer :");
					
				    }
			    }
		    }
	    }


	if(definitionNS==null)
	    {
		if(tns!=null)
		    {
			Hashtable nsHt = new Hashtable();
			nsHt.put("xmlns:gsoapDefNS",tns);
			definitionNS = getNS("gsoapDefNS:",nsHt);
			System.out.println("Warning : Definition Name Space not found choosing "+ definitionNS +" as definition Name Space\nThis MAY cause problems!!!!");
			globalNameSpaces.put("xmlns:gsoapDefNS",tns);
			defans = definitionNS;
		    }
		else
		    System.out.println("Warning : Definition Name Space is NULL");
	    }
	
    }



    Document replaceImportNodes(Document node )
    {
	DOMParser parser = new DOMParser(); 
	NodeSearch importSearch = new NodeSearch(node,"import");
	Node importNode = importSearch.getNextPartialMatch();
	boolean newDocImported = false;
	
	try{
	for(int i=0;i<importSearch.getTotalMatches();i++)
	    {
		Node parent = importNode.getParentNode();

		String loc = getAttrValue(importNode, "location");

		Document importDoc=null;
		if(loc!=null)
		    {
			System.out.println("Importing document "+loc);
			parser.parse(loc);
			importDoc  = parser.getDocument();

			Node n = node.importNode(importDoc.getDocumentElement(),true);

			parent.insertBefore(n,importNode);
			parent.removeChild(importNode);

			newDocImported = true;
		    }
		importNode = importSearch.getNextNode();
	    }

							
	    } 
	// catch the excepetion
	catch (IOException e) {
            System.out.println("Error reading URI: " + e.getMessage());
	    System.exit(0);
        } catch (SAXException e) {
            System.out.println("Error in parsing: " + e.getMessage());
	    System.exit(0);
        }
	if(!newDocImported)
	    {
		return node;
	    }
	else
	    return replaceImportNodes(node);
    }




    /*call the parsing process by creating hte document object */
    public void gsoapParser(String uri,boolean c)
    {
	outc = c;

	uri=uri.trim();
	
	System.out.println("/*Parsing XML File: " + uri + "*/");       
        
        // Instantiate your vendor's DOM parser implementation
        DOMParser parser = new DOMParser(); 
        try
	    {
		parser.parse(uri);
		
		Document doc = parser.getDocument();           
		
		doc= replaceImportNodes(doc );
		parseDoc(doc);
		
		/*write .h file*/
		String outfilename="outfile.h";/* = uri.substring(0,uri.indexOf('.'))+".h";*/
		String fname=uri;
		if(uri.startsWith("http:"))
		    {
			fname = uri.substring(uri.lastIndexOf('/')+1);
			fname = fname.substring(fname.indexOf('?')+1);
			fname = fname.substring(fname.indexOf('=')+1);
		    }
		
		if(fname.indexOf('.')!=-1)
		    {
			fname = fname.substring(0,fname.indexOf('.'));
		    }
		
		outfilename = fname+".h";
		
		FileWriter fw = new FileWriter(outfilename);
		PrintWriter pw = new PrintWriter(fw);
		
		System.out.println("Processing Done.\n\t\t Header File\t\t"+outfilename);
		
		NsNodeSearch ndefinition = new NsNodeSearch(doc,docNS+"definitions",new Hashtable());
		NsNode ndefinitionNode = ndefinition.getNextNode();
		
		Hashtable namespaces = ndefinitionNode.getNameSpace();
		
		Enumeration keyList;
		String key;
		String twsdlns = getNsEquivalent(namespaces,"wsdlNS");
		String txsdns = getNsEquivalent(namespaces,"xsdNS");
		String tsoapns = getNsEquivalent(namespaces,"soapNS");
		String defaultns = getNsEquivalent(namespaces,"definitionNS");
		
		/*populate the printed classes vector as the basic types will
		 * be printed if present*/
		printedClasses.add("xsd:base64Binary");
		printedClasses.add("xsd:hexBinary");
		printedClasses.add("soap_enc:hexBinary");
		printedClasses.add("soap_enc:base64Binary");
		String name;
		
		
		keyList = bindingsList.keys();
		while(keyList.hasMoreElements())
		    {
			key = (String)keyList.nextElement();
			
			pw.println("//gsoap "+(String)bindingsList.get(key)+" schema namespace: "+ key);
		    }
		
		keyList = namespaces.keys();
		while(keyList.hasMoreElements())
		    {
			key = (String)keyList.nextElement();
			name = getName(key)+":";
			
			if(key.startsWith("xmlns:"))
			    {
				if(!(name.equals(twsdlns)||name.equals(txsdns)||name.equals(tsoapns)||name.equals(defaultns)))
				    {
					pw.println("//gsoap "+ getName(key)+" schema namespace: "+(String)namespaces.get(key));
				    }
			    }
		    }
		
		pw.println("");
		
		if(defaultns!=null)
		    {
			defaultns = defaultns.substring(0,defaultns.length()-1);
			pw.println("//gsoap "+ defaultns + " service namespace: "+namespaces.get("xmlns:"+defaultns));
		    }
		pw.println("");
		
		pw.println("//gsoap "+defaultns+" service location: "+serviceAddrLocation);
		pw.println("//gsoap "+defaultns+" service name: "+"soap"+serviceName);
		
		pw.println("\n/*start primitive data types*/");
		
		keyList = allPrimitiveXSDDataType.keys();
		
		while(keyList.hasMoreElements())
		    {
			key=(String)keyList.nextElement();
			
			if(((Boolean)allPrimitiveXSDDataType.get(key)).booleanValue())
			    pw.print(allPrimitiveXSDDataTypeEquivalent.get(key));
			
		    }
		
		keyList = allPrimitiveSOAPDataType.keys();
		
		while(keyList.hasMoreElements())
		    {
			key=(String)keyList.nextElement();
			
			if(((Boolean)allPrimitiveSOAPDataType.get(key)).booleanValue())
			    pw.print(allPrimitiveSOAPDataTypeEquivalent.get(key));
			
		    }

		pw.println("\n/*end primitive data types*/\n");
		
		/*print header information*/
		keyList = soapHeaders.elements();

		if(keyList.hasMoreElements())
		    {
			if(!outc)
			    pw.println("class SOAP_ENV__Header  {\n    public:");
			else
			    pw.println("struct SOAP_ENV__Header  {\n");
			
			String headerNSStr="";
			if(headerNS!=null) headerNSStr= headerNS.substring(0,headerNS.length()-1)+"__";

			while(keyList.hasMoreElements())
			    {
				String msg = (String)keyList.nextElement();
				StringTokenizer lineToken = new StringTokenizer(msg,"\n");
				
				while(lineToken.hasMoreElements())
				    {
					
					StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
					String dType = (String)tabToken.nextElement();
					String ptr = "*";
					
					if(Integer.parseInt((String)tabToken.nextElement())==1)
					    ptr = "*";
					String varName = (String)tabToken.nextElement();
					int dataType = Integer.parseInt((String)tabToken.nextElement());
					
					switch(dataType)
					    {
					    case 0://xsd type
						pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
						break;
					    case 1://classes
						if(!outc)
						    {
							if(printedClasses.contains(dType))
							    pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
							else
							    pw.print("\tclass "+dataType(dType)+" "+ptr+" "+varName+";\n");
						    }
						else
						    pw.print("\tstruct "+dataType(dType)+" * "+varName+";\n");
						break;
					    case 2://enumerations
						pw.print("\tenum "+dataType(dType)+" "+ptr+" "+varName+";\n");
						break;
						
					    case 3://print without any modifications (reserved mainly for arrays)
						pw.print(dType+" "+ptr+" "+varName+";\n");
						break;
					    }
				    }
				
			    }
			pw.println("\t};");
		    }
		
		//System.out.println(allDataType);
		String dataName;
		String description;		

		//new implementation to print data types
		if(!outc)
		    {
			Hashtable extendedClasses = new Hashtable();

			keyList = allDataType.keys();
			
			while(keyList.hasMoreElements())
			    {
				dataName = (String)keyList.nextElement();
				description= (String)allDataType.get(dataName);
				
				StringTokenizer lineToken = new StringTokenizer(description,"\n");
				
				int dataType = Integer.parseInt((String)lineToken.nextElement());
				
				switch(dataType)
				    {
				    case 1://MASK
					pw.println("enum * "+dataType(dataName)+" { "+description.substring(description.indexOf('\n')+1)+" };\n");
					break;
				    case 2://ENUM
					pw.println("enum "+dataType(dataName)+" { "+description.substring(description.indexOf('\n')+1)+" };\n");
					break;
				    case 4://array same as class for now 
				    case 3://class
					printedClasses.add(dataName);
					pw.println("class "+dataType(dataName)+" {\n   public: ");
					while(lineToken.hasMoreElements())
					    {
						StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
						String ptr = "";
						String dType = (String)tabToken.nextElement();

						if(Integer.parseInt((String)tabToken.nextElement())==1)
						    ptr="*";
						
						String varName = (String)tabToken.nextElement();
						int type = Integer.parseInt((String)tabToken.nextElement());

						switch(type)
						    {
						    case 0://xsd type
							pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
							break;
						    case 1://classes
							if(printedClasses.contains(dType))
							    pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
							else
							    pw.print("\tclass "+dataType(dType)+" "+ptr+" "+varName+";\n");
							break;
						    case 2://enumerations
							pw.print("\tenum "+dataType(dType)+" "+ptr+" "+varName+";\n");
							break;
							
						    case 3://print as such (reserve)
							pw.print("\t"+dType+" "+ptr+" "+varName+";\n");
							break;
						    }
					    }
					pw.println("};\n");
					break;
				    case 5://extended classes
					extendedClasses.put(dataName,description);
					break;
				    case 6://typedef
					printedClasses.add(dataName);
					StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
					String dType=(String)tabToken.nextElement();

					pw.println("typedef "+dataType(dType)+"  "+dataType(dataName)+" ;\n");
					break;
				    }
			    }
			printExtendedClasses(extendedClasses,pw);		
			
		    }
		else
		    {		    
			//normal c implementation
			
			Hashtable extendedClasses = new Hashtable();

			keyList = allDataType.keys();
			
			while(keyList.hasMoreElements())
			    {
				dataName = (String)keyList.nextElement();
				description= (String)allDataType.get(dataName);
				
				StringTokenizer lineToken = new StringTokenizer(description,"\n");
			
				int dataType = Integer.parseInt((String)lineToken.nextElement());
				
				switch(dataType)
				    {
				    case 1://MASK
					pw.println("enum * "+dataType(dataName)+" { "+description.substring(description.indexOf('\n')+1)+" };\n");
					break;
				    case 2://ENUM
					pw.println("enum "+dataType(dataName)+" { "+description.substring(description.indexOf('\n')+1)+" };\n");
					break;
				    case 4://array same as class for now 
				    case 3://class
					pw.println("struct "+dataType(dataName)+" {");
					while(lineToken.hasMoreElements())
					    {
						StringTokenizer tabToken = new
						    StringTokenizer((String)lineToken.nextElement(),"\t");
						String ptr = "";
						String dType = (String)tabToken.nextElement();
						
						if(Integer.parseInt((String)tabToken.nextElement())==1)
						    ptr="*";
						String varName = (String)tabToken.nextElement();
						int type = Integer.parseInt((String)tabToken.nextElement());
						
						switch(type)
						    {
						    case 0://xsd type
							pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
							break;
						    case 1://classes
							pw.print("\tstruct "+dataType(dType)+" * "+varName+";\n");
							break;
						    case 2://enumerations
							pw.print("\tenum "+dataType(dType)+" "+ptr+" "+varName+";\n");
						break;
						
						    case 3://print as such (reserve)
							pw.print("\t"+dType+" "+ptr+" "+varName+";\n");
							break;
						    }
					    }
					pw.println("};\n");
					break;
				    case 5:
					extendedClasses.put(dataName,description);
					break;
				    case 6://typedef
					printedClasses.add(dataName);
					StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
					String dType =(String)tabToken.nextElement();
					pw.println("typedef "+dataType(dType)+"  "+dataType(dataName)+" ;\n");
					break;
				    }
			    }
		    
			printExtendedClassesInC(extendedClasses,pw);	

		    }

		/*had a big block here*/

		//new operations handling

		keyList = allOperation.keys();
		
		while(keyList.hasMoreElements())
		    {
			key = (String)keyList.nextElement();
			
			pw.println("//gsoap "+getNS(key).substring(0,getNS(key).length()-1)+ " service method-action: "+convertToCpp(getName(key),false)+" \""+(String)soapActions.get(key)+"\"");
			
			
			pw.print(dataType(key)+"( ");
			
			description = (String)allOperation.get(key);
			StringTokenizer lineToken = new StringTokenizer(description,"\n");
			
			while(lineToken.hasMoreElements())
			    {
				StringTokenizer tabToken = new
				    StringTokenizer((String)lineToken.nextElement(),"\t");
				String ptr = "";
				String dType = (String)tabToken.nextElement();
				if(Integer.parseInt((String)tabToken.nextElement())==1)
				    ptr="*";
				String varName = (String)tabToken.nextElement();
				int type = Integer.parseInt((String)tabToken.nextElement());
				
				switch(type)
				    {
				    case 0://Basic 
					pw.print(dataType(dType)+" "+ptr+" "+varName);
					break;
				    case 1://class
					if(!outc)
					    pw.print(dataType(dType)+" "+ptr+" "+varName);
					else
					    pw.print("struct "+dataType(dType)+" "+ptr+" "+varName);
					break;
				    case 2://enum
					pw.print("enum "+dataType(dType)+" "+ptr+" "+varName);
					break;
					
				    case 3://print as such
					pw.print(dType+" "+ptr+" "+varName);
					break;
				    }
				if(lineToken.hasMoreElements())
				    pw.print(", ");
			    }
				pw.println(" );");
		    }
		pw.close();		
		//System.out.println(allDataType);
		
		
		/*write .c file*/
		
		outfilename =fname+".c"; /*uri.substring(0,uri.indexOf('.'))+".c";*/
			
		fw = new FileWriter(outfilename);
		pw = new PrintWriter(fw);
		System.out.println("\t\t Sample C File\t\t"+outfilename);
		pw.println("#include \"soapH.h\"");
		pw.println("#include \""+"soap"+serviceName+".nsmap\"");
		pw.println("main()");
		pw.println("{\n\tstruct soap soap;");
		pw.println("\tsoap_init(&soap);");
		pw.println("\n");
		/*keyList = allOperation.keys();
		String functionName="";
		String parameters="";
		int index1=0,index2=0;
		String saction="";
		
		while(keyList.hasMoreElements())
		    {
			key = (String)keyList.nextElement();
			description = (String)allOperation.get(key);

			saction = (String)soapActions.get(key);

			index1 = description.indexOf('(');
			index2= description.lastIndexOf(')');
			functionName = description.substring(0,index1).trim();
			parameters = description.substring(index1+1,index2).trim();

			pw.print("\tif (soap_call_"+functionName+" ( &soap, \""+serviceAddrLocation+"\", \"");
			pw.println(saction+"\",/* "+parameters+"*//*))");
			pw.println("\t\tsoap_print_fault(&soap,stderr);\n\n");
			
		    }
								  */
		    
		//new operations handling
		keyList = allOperation.keys();
		while(keyList.hasMoreElements())
		    {
			key = (String)keyList.nextElement();
			
			pw.print("\tif (soap_call_"+dataType(key)+" ( &soap, \""+serviceAddrLocation+"\", \""+(String)soapActions.get(key)+"\",/* ");


			description = (String)allOperation.get(key);
			StringTokenizer lineToken = new StringTokenizer(description,"\n");
			
			while(lineToken.hasMoreElements())
			    {
				StringTokenizer tabToken = new
				    StringTokenizer((String)lineToken.nextElement(),"\t");
				String ptr = "";
				String dType = (String)tabToken.nextElement();
				if(Integer.parseInt((String)tabToken.nextElement())==1)
				    ptr="*";
				String varName = (String)tabToken.nextElement();
				int type = Integer.parseInt((String)tabToken.nextElement());
				
				switch(type)
				    {
				    case 0://Basic 
					pw.print(dataType(dType)+" "+ptr+" "+varName);
					break;
				    case 1://class
					if(!outc)
					    pw.print(dataType(dType)+" "+ptr+" "+varName);
					else
					    pw.print("struct "+dataType(dType)+" "+ptr+" "+varName);
					break;
				    case 2://enum
					pw.print("enum "+dataType(dType)+" "+ptr+" "+varName);
					break;
					
				    case 3://print as such
					pw.print(dType+" "+ptr+" "+varName);
					break;
				    }
				if(lineToken.hasMoreElements())
				    pw.print(", ");
			    }
			pw.println("*/ ))");
			pw.println("\t\tsoap_print_fault(&soap,stderr);\n\n");
		    }
		pw.println("}");
		pw.close();

    }
	// catch the excepetion
	catch (IOException e) {
            System.out.println("Error reading URI: " + e.getMessage());
        } catch (SAXException e) {
            System.out.println("Error in parsing: " + e.getMessage());
        }
    }

    void printExtendedClasses(Hashtable extendedClasses, PrintWriter pw)
    {
	Enumeration keyList = extendedClasses.keys();
	String dataName;
	String description;
	
	while(keyList.hasMoreElements())
	    {
		dataName = (String)keyList.nextElement();
		description= (String)allDataType.get(dataName);
		StringTokenizer lineToken = new StringTokenizer(description,"\n");
		 
		int structType = Integer.parseInt((String)lineToken.nextToken());
		
		if(structType==5)
		    {
			printedClasses.add(dataName);

			pw.print("class "+dataType(dataName)+" : public ");

			String baseName =  (String)lineToken.nextElement();
			StringTokenizer tabToken = new StringTokenizer(baseName,"\t");

			baseName = (String)tabToken.nextElement();
			pw.println(dataType(baseName)+" { ");
			
			int linesToSkip =Integer.parseInt((String)tabToken.nextElement());

			while(linesToSkip--!=0)lineToken.nextElement();
			
			while(lineToken.hasMoreElements())
			    {
				tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
				String ptr = "";
				String dType = (String)tabToken.nextElement();
				
				if(Integer.parseInt((String)tabToken.nextElement())==1)
				    ptr="*";
				String varName = (String)tabToken.nextElement();
				int type = Integer.parseInt((String)tabToken.nextElement());
				
				switch(type)
				    {
				    case 0://xsd type
					pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
					break;
				    case 1://classes
					if(printedClasses.contains(dType))
					    pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
					else
					    pw.print("\tclass "+dataType(dType)+" "+ptr+" "+varName+";\n");
					break;
				    case 2://enumerations
					pw.print("\tenum "+dataType(dType)+" "+ptr+" "+varName+";\n");
					break;
					
				    case 3://print as such (reserve)
					pw.print("\t"+dType+" "+ptr+" "+varName+";\n");
					break;
				    }
			    }
			pw.println("};\n");
		    }
	    }
    }




    void printExtendedClassesInC(Hashtable extendedClasses, PrintWriter pw)
    {
	Enumeration keyList = extendedClasses.keys();
	String dataName;
	String description;
	
	while(keyList.hasMoreElements())
	    {
		dataName = (String)keyList.nextElement();
		description= (String)allDataType.get(dataName);
		StringTokenizer lineToken = new StringTokenizer(description,"\n");
		 
		int structType = Integer.parseInt((String)lineToken.nextToken());
		
		if(structType==5)
		    {
			printedClasses.add(dataName);

			pw.println("struct "+dataType(dataName)+" {");

			String baseName =  (String)lineToken.nextElement();
			
			while(lineToken.hasMoreElements())
			    {
				StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
				String ptr = "";
				String dType = (String)tabToken.nextElement();
				
				if(Integer.parseInt((String)tabToken.nextElement())==1)
				    ptr="*";
				String varName = (String)tabToken.nextElement();
				int type = Integer.parseInt((String)tabToken.nextElement());
				
				switch(type)
				    {
				    case 0://xsd type
					pw.print("\t"+dataType(dType)+" "+ptr+" "+varName+";\n");
					break;
				    case 1://classes
					pw.print("\tstruct "+dataType(dType)+" * "+varName+";\n");
					break;
				    case 2://enumerations
					pw.print("\tenum "+dataType(dType)+" "+ptr+" "+varName+";\n");
					break;
					
				    case 3://print as such (reserve)
					pw.print("\t"+dType+" "+ptr+" "+varName+";\n");
					break;
				    }
			    }
			pw.println("};\n");
		    }
	    }
    }

    

    /*convert the datatype ie convert : to __*/
    private String dataType(String str)
    {
	if (str==null){System.out.println("Data type is found to be null!!!!");System.exit(0);}

	StringBuffer finalString = new StringBuffer();
	
	StringTokenizer colon = new StringTokenizer(str,":",true);
	String temp;


	while(colon.hasMoreElements())
	    {
		temp= (String)colon.nextElement();
		if(temp.equals(":"))
		    finalString.append("__");
		else
		    finalString.append(convertToCpp(temp,false));
	    }


	String uscores="";
	if(keyWords.contains(finalString.toString()))
	    uscores="_";
	return finalString.toString()+uscores;
	
	//return finalString.toString();
    }


    /*convert the string to print format _ , . and __ */

    private String convertToCpp(String str,boolean uscoreflag)
    {
	if(str==null)str =" ";
	StringBuffer finalString = new StringBuffer();
	//replace  all '_' with _USCORE_ this should be done first as all
	// other replacements will add'_' to the string

       
	StringTokenizer uscore = new StringTokenizer(str,"_",true);
	String temp;
	//	finalString.append((String)uscore.nextElement());
	while(uscore.hasMoreElements())
	    {
		temp = (String)uscore.nextElement();
		if(temp.equals("_"))
		    finalString.append("_USCORE_");
		else
		    finalString.append(temp);
	    }

	//replace all '.' with '_DOT_'
	StringTokenizer dot = new StringTokenizer(finalString.toString(),".",true);
	finalString = new StringBuffer();

	//finalString.append((String)dot.nextElement());
	while(dot.hasMoreElements())
	    {
		temp = (String)dot.nextElement();
		if(temp.equals("."))
		    finalString.append("_DOT_");
		else
		    finalString.append(temp);
	    }

	//replace all '-' with '_'
	StringTokenizer hyphen = new StringTokenizer(finalString.toString(),"-",true);
	finalString = new StringBuffer();


	//finalString.append((String)hyphen.nextElement());
	while(hyphen.hasMoreElements())
	    {
		temp = (String)hyphen.nextElement();
		if(temp.equals("-"))
		    finalString.append("_");
		else
		    finalString.append(temp);
	    }

	String uscores="";
	if(keyWords.contains(finalString.toString())&&uscoreflag)
	    uscores="_";
	return finalString.toString()+uscores;
    }
    
    /*get the value of attribute of the given node*/

    public String getAttrValue(Node node, String attribute)
    {
	NamedNodeMap attributes = node.getAttributes();
	for (int i=0; i<attributes.getLength(); i++)
	    {
		Node current = attributes.item(i);
		if(current.getNodeName().compareToIgnoreCase(attribute)==0)
		    {
			return current.getNodeValue();
		    }
		}
	return null;
    }
    

private String serviceName=null;
private String serviceAddrLocation=null;
private String servicePortName=null;
private String servicePortBinding=null;



    public boolean getServiceInfo(Node node)
    {
	Node serviceNode=null;
	Node portNode=null;
	Node soapAddrNode=null;
	/*
	String docNS = (String) namespace.get("docNS");
	String soapNS = (String) namespace.get("soapNS");
	*/
	NodeSearch service = new NodeSearch(node,docNS+"service");

	serviceNode = service.getNextNode();
	
	if(serviceNode==null)
	    {
		if(defWsdl)
		    {
			service = new NodeSearch(node,"service");
		    	serviceNode = service.getNextNode();
		    }
		if(serviceNode==null){
		System.out.println("Service element not found");
		return false;
		}
	    }
	
	serviceName = getAttrValue(serviceNode,"name");
	if(serviceName == null)
	    {
		System.out.println("name attribute of service not found");
		return false;
	    }

	portNode = (new NodeSearch(serviceNode,docNS+"port")).getNextNode();
	if(portNode == null)
	    {
		if(defWsdl)
		    portNode = (new NodeSearch(serviceNode,"port")).getNextNode();
		if(portNode == null)
		    {
		System.out.println("element PORT not found");
		return false;}
	    }

	servicePortName = getAttrValue(portNode,"name");
	if(servicePortName == null)
	    {
		System.out.println("name attribute of PORT not found");
		return false;
	    }

	servicePortBinding = getAttrValue(portNode,"binding");
	if(servicePortBinding == null)
	    {
		System.out.println("Binding attribute of PORT not found");
		return false;
	    }

	soapAddrNode = (new NodeSearch(serviceNode,soapNS+"address")).getNextNode();
	if(soapAddrNode == null)
	    {
		if(defSoap)
		soapAddrNode = (new
		    NodeSearch(serviceNode,"address")).getNextNode();
		if(soapAddrNode == null)
		    {
			System.out.println("element SOAP ADDRRESS not found");
		return false;}
	    }

	serviceAddrLocation = getAttrValue(soapAddrNode,"location");
	if(serviceAddrLocation == null)
	    {
		System.out.println("Location attribute of SOAP ADDRESS not found");
		return false;
	    }
	    return true;
    }

private String operationName=null;
private String inputName=null;
private String outputName=null;

public boolean getBindingInfo(Node node, String bindingName)
    {
	Node bindingNode=null;
	Node operationNode=null;
	Node inputNode=null;
	Node outputNode=null;

	Hashtable attrib = new Hashtable();

	attrib.put("name",bindingName);
	NodeSearch binding = new NodeSearch(node,docNS+"binding",attrib);

	Hashtable allHeaders = new Hashtable();

	
	bindingNode = binding.getNextNode();
	
	if(bindingNode == null)
	    {
		if(defWsdl)
		    {
			binding = new NodeSearch(node,"binding",attrib);
		    	bindingNode = binding.getNextNode();
		    }
		if(bindingNode == null){
		    System.out.println("element BINDING not found");
		return false;}
	    }

	String bindingType = getAttrValue(bindingNode,"type");

	bindingType = bindingType.substring(bindingType.indexOf(":")+1);
	

	NodeSearch operation = new NodeSearch(bindingNode,docNS+"operation");

	operationNode = operation.getNextNode();
	if(operationNode == null)
	    {
		if(defWsdl){
		    operation = new NodeSearch(bindingNode,"operation");
		    operationNode = operation.getNextNode();}
		
		if(operationNode == null){
		System.out.println("element OPERATION not found");
		return false;}
	    }
	
	while(operationNode!=null)
	    {
		operationName = getAttrValue(operationNode,"name");

		if(operationName == null)
		    {
			System.out.println("name attribute of OPERATION not found");
			return false;
		    }

		NodeSearch soapoperation = new NodeSearch(operationNode,soapNS+"operation");
		Node soapOperationNode = soapoperation.getNextNode();

		if((soapOperationNode==null)&&(defSoap))
		    {
			soapoperation = new NodeSearch(operationNode,"operation");
			soapOperationNode = soapoperation.getNextNode();
		    }

		
		String soapact="";
		if(soapOperationNode!=null)
		    {
			soapact = getAttrValue(soapOperationNode,"soapAction");
			//System.out.println("soap action "+getAttrValue(soapOperationNode,"soapAction"));
		    }



		NodeSearch soapheader = new NodeSearch(operationNode,soapNS+"header");
		Node soapHeaderNode = soapheader.getNextNode();

		if((soapHeaderNode==null)&&(defSoap))
		    {
			soapheader = new NodeSearch(operationNode,"header");
			soapHeaderNode = soapoperation.getNextNode();
		    }
		
		while(soapHeaderNode!=null)
		    {
			String headerEncoding = getAttrValue(soapHeaderNode,"encodingStyle");
			String headerMessage = getName(getAttrValue(soapHeaderNode,"message"));

			if((headerEncoding!=null)&&(headerMessage!=null))
			    allHeaders.put(headerMessage,headerEncoding);
			else
			    System.out.println("Warning : Header information for the operation "+operationName+ " is incomplete \n"+"Found encodingStyle :"+ headerEncoding+ "   message :"+headerMessage);
			soapHeaderNode = soapoperation.getNextNode();
		    }

		
		NodeSearch soapbody = new NodeSearch(bindingNode,soapNS+"body");
		Node soapBodyNode = soapbody.getNextNode();

		if((soapBodyNode==null)&&(defSoap))
		    {
			soapbody = new NodeSearch(bindingNode,"body");
			soapBodyNode = soapbody.getNextNode();
		    }

		String tdefns;

		if(definitionNS!=null)
		    tdefns = definitionNS.substring(0,definitionNS.length()-1);
		else
		    tdefns = "";

		if(soapBodyNode!=null)
		    {
			String tnsp = getAttrValue(soapBodyNode,"namespace");

			if(tnsp!=null){
			if(bindingsList.containsKey(tnsp))
			    tdefns=(String)bindingsList.get(tnsp);
			else
			    {
				if(!bindingsList.containsValue(tdefns))
				    {
					bindingsList.put(tnsp,tdefns);
				    }
				else
				    {
					int cnt=1;
					while(bindingsList.containsValue(tdefns+cnt))cnt++;
					bindingsList.put(tnsp,tdefns+cnt);
					tdefns+=cnt;
				    }
			    }
			}
		    }
		
		getOperationInfo(node,bindingType,operationName,tdefns+":",soapact);
		
		operationNode = operation.getNextNode();
	    }

	//process soap headers
	Enumeration keyList;
	String key;
	String messageEquivalent;
	String dataType;
	String dataName;
	
	keyList = allHeaders.keys();

	String member;

	while(keyList.hasMoreElements())
	    {
		key = (String)keyList.nextElement();
		messageEquivalent = getMessageEquivalent(node,key,false).trim();
		
		soapHeaders.add(messageEquivalent);

		/*StringTokenizer stMembers = new StringTokenizer(messageEquivalent,"\n");
		while(stMembers.hasMoreElements())
		    {
			member = ((String)stMembers.nextElement()).trim();
			StringTokenizer stType = new StringTokenizer(member,"\t");

			dataType = ((String)stType.nextElement()).trim();
						if(dataType.compareToIgnoreCase("struct")==0)
				dataType += " "+ ((String)stType.nextElement()).trim();
			

			String ptr = "";
			if(Integer.parseInt((String)stType.nextElement())==1)
			    ptr = "* ";
			
			dataName = ((String)stType.nextElement()).trim();

			if(Integer.parseInt((String)stType.nextElement())==1)
			while(soapHeaders.containsKey(dataName))dataName+= "_";
			
			soapHeaders.put(dataName,dataType);			    

			}*/
	    }
	/*System.out.println("*****************End Operation Information*********************************");*/
	return true;
    }

    public void getOperationInfo(Node node,String bindingType,String
				   operationName,String defns,String soapact)
    {
	Hashtable bindattrib = new Hashtable();
	Hashtable operattrib = new Hashtable();

	bindattrib.put("name",bindingType);

	operattrib.put("name",operationName);

	NsNodeSearch bindings = new NsNodeSearch(node,docNS+"portType",bindattrib,new Hashtable());
	NsNode binding = bindings.getNextNode();

	NsNodeSearch operations = new NsNodeSearch(binding.getNode(),docNS+"operation",operattrib,binding.getNameSpace());
    
					       
	NsNode noperation = operations.getNextNode();
	Node operation = noperation.getNode();
	
	if((operation==null)&&(defWsdl))
	    {
		bindings = new NsNodeSearch(node,"portType",bindattrib,new Hashtable());
		binding = bindings.getNextNode();
		operations = new NsNodeSearch(binding.getNode(),"operation",operattrib,binding.getNameSpace());
		noperation = operations.getNextNode();
		operation = noperation.getNode();
	    }

	while(noperation!=null)
	    {
		operation = noperation.getNode();
		
		if(operation == null)
		    {
			System.out.println("operation NOT found "+operationName);
			return;
		    }

		elementflag = false;
		
		NsNodeSearch input = new NsNodeSearch(operation,docNS+"input",noperation.getNameSpace());
		NsNode ninputNode = input.getNextNode();

		Node inputNode = ninputNode.getNode();


		if((inputNode==null)&&(defWsdl))
		    {
			input = new NsNodeSearch(operation,"input",noperation.getNameSpace());
			ninputNode = input.getNextNode();
			inputNode = ninputNode.getNode();
		    }

		
		StringBuffer inputArgs = new StringBuffer();
		StringBuffer outArgs = new StringBuffer();

		
		String messageEquivalent=null;
		for(int i=0;i<input.getTotalMatches();i++)
		    {
			if(inputNode==null)
			    {
				System.out.println("INput node for operation " +
						   convertToCpp(operationName,true)+" Not found");
				return;
			    }
			
			
			String inmessage = getAttrValue(inputNode,"message");
			inmessage = inmessage.substring(inmessage.indexOf(":")+1);

			messageEquivalent = getMessageEquivalent(node,inmessage,false);
			
			inputArgs.append(messageEquivalent);
			ninputNode = input.getNextNode();
			if(ninputNode!=null)
			    inputNode = ninputNode.getNode();
		    }


		NodeSearch output = new NodeSearch(operation,docNS+"output");
		Node outputNode = output.getNextNode();
		if((outputNode==null)&&(defWsdl))
		    {
			output = new NodeSearch(operation,"output");
			outputNode = output.getNextNode();
		    }
		
		for(int i=0;i<output.getTotalMatches();i++)
		    {
			if(outputNode==null)
			    {
				System.out.println("outputput node for operation " +
						   convertToCpp(operationName,true)+" Not found");
				return;
			    }
			
			String outmessage = getAttrValue(outputNode,"message");
			outmessage = outmessage.substring(outmessage.indexOf(":")+1);
			
			messageEquivalent = getMessageEquivalent(node,outmessage,true);
			
			outArgs.append(messageEquivalent);
			outputNode = output.getNextNode();
			
		    }

		String description="";
		String returnDataType;
		
		if(elementflag)
		    operationName = operationName+"-";


		returnDataType = addDataType(defns+operationName+"Response",3,outArgs.toString().trim());
		description=  inputArgs+returnDataType+"\t1\tout\t1\n";


		addOperation(defns,operationName,description,soapact);

		noperation = operations.getNextNode();
		if(noperation==null)
		    operation = null;
		else
		    operation = noperation.getNode();
	    }
	
	
    }
    

    Hashtable allOperation = new Hashtable();

    void addOperation(String ldefNS,String operationName,String
		      description,String soapaction)
    {
	boolean inserted=false;
	String opString="";

	while(!inserted)
	    {
		if(!allOperation.containsKey(ldefNS+operationName+opString))
		    {
			allOperation.put(ldefNS+operationName+opString,description);
			soapActions.put(ldefNS+operationName+opString,soapaction);
			inserted=true;
		    }
		else
		    {
			int pos;
			
			if(((String)allOperation.get(ldefNS+operationName+opString)).compareToIgnoreCase(description)==0)
			    return;

			opString = opString+"-";
		    }
	    }
		
    }
    
   String getNsEquivalent(Hashtable ht, String name)
    {
	if(ht==null)
	    return null;
	else
	    return (String)ht.get(name);
    }

    boolean isdefWsdl(Hashtable ht)
    {
	String sdefaultNS = (String)ht.get("defaultNS");
	if(sdefaultNS!=null)
	    {
		if(sdefaultNS.endsWith("/wsdl/"))
		    {
			return true;
		    }

	    }
	return false;
    }
    boolean isdefSoap(Hashtable ht)
    {
	String sdefaultNS = (String)ht.get("defaultNS");
	if(sdefaultNS!=null)
	    {
		if(sdefaultNS.endsWith("/soap/"))
		    {
			return true;
		    }

	    }
	return false;
    }

    
    private boolean elementflag;    

String getMessageEquivalent(Node node, String messageName,boolean isreturn)
    {
	Node messageNode=null;

	StringBuffer equivalentString = new StringBuffer();

	StringBuffer cppEquivalentString = new StringBuffer();
	
	Hashtable attr = new Hashtable();

	attr.put("name",messageName);
	
	NsNodeSearch nmessage = new NsNodeSearch(node,docNS+"message",attr,new Hashtable());
	NsNode nmessageNode = nmessage.getNextNode();

	if((nmessageNode==null)&&(defWsdl))
	    {
		 nmessage = new NsNodeSearch(node,"message",attr,new Hashtable());
		 nmessageNode = nmessage.getNextNode();
	    }
	elementflag = false;

	String uscore="";
	if(isreturn)
	    uscore="_";

	if(nmessageNode!=null)	
	messageNode= nmessageNode.getNode();
	else{
	    System.out.println("message of message name" +messageName+
				   "not found");
	    System.exit(0);
	}

	NsNodeSearch part = new NsNodeSearch(messageNode,docNS+"part",nmessageNode.getNameSpace());

	//StringBuffer messageEq = new StringBuffer();
	NsNode npartNode=part.getNextNode();

	if((npartNode==null)&&(defWsdl))
	    {
		part = new NsNodeSearch(messageNode,"part",nmessageNode.getNameSpace());
		npartNode=part.getNextNode();
	    }
	
	Node partNode=null;
	if(npartNode!=null)
	    partNode = npartNode.getNode();

	for(int i=0;i<part.getTotalMatches();i++)
	    {
		String partName=null;
		String partType=null;
		String partElement=null;

		String cppPartType=null;
		
		partName = getAttrValue(partNode,"name");
		partType = getAttrValue(partNode,"type");
		partElement = getAttrValue(partNode,"element");/*not need in
								* soap encoding*/
		
		if(partName == null)
		    System.out.println("name field for part is NOT found");

		String xsdNS = getNsEquivalent(npartNode.getNameSpace(),"xsdNS");
		String soapNS = getNsEquivalent(npartNode.getNameSpace(),"soapNS");
		String encodingNS = getNsEquivalent(npartNode.getNameSpace(),"encodingNS");

				
		if(partType == null)//means it is specfied in other way (element)
		    {
			if(partElement!=null)//type of element is specified 
			    {
				int isStruct=0;
				
				elementflag = true;
				String eleEq;

				if(isXsd(partElement,xsdNS))
				    {
					partType ="xsd:"+getName(partElement);

					//special condition for binary
					if((getName(partElement).compareToIgnoreCase("hexBinary")==0)||
					   (getName(partElement).compareToIgnoreCase("base64Binary")==0))
					    {
						//partType = getName(partElement);
						isStruct=1;
					    }
					eleEq = partType+ "\t"+isStruct+"\t" +uscore+convertToCpp(partName,true)+ "\t"+isStruct+"\n";
				    }
				else if(isSoapEnc(partElement,encodingNS))
				    {
					partType = "soap-enc:"+getName(partElement);
					if((getName(partElement).compareToIgnoreCase("hexBinary")==0)||
					   (getName(partElement).compareToIgnoreCase("base64Binary")==0))
					    {
						//partType = getName(partElement);
						isStruct=1;
					    }
					eleEq = partType+ "\t"+isStruct+"\t" +uscore+convertToCpp(partName,true)+ "\t"+isStruct+"\n";
				    }
				else
				    {
					eleEq = (String)allDataType.get(partElement);

					if(eleEq==null) eleEq = "";
					else
					    {
						StringTokenizer st = new StringTokenizer(eleEq,"\n");
						int dataStructureType = Integer.parseInt(((String)st.nextElement()).trim());
						if(dataStructureType==5)//extended classes
						    {
							eleEq = eleEq.substring(eleEq.indexOf('\n')+1);
							eleEq = eleEq.substring(eleEq.indexOf('\n')+1);
						    }
						else if(dataStructureType==6)//typeDef
						    {
							StringTokenizer tabToken = new StringTokenizer((String)st.nextElement(),"\t");

							eleEq = tabToken.nextElement()+ "\t0\t" +uscore+convertToCpp(partName,true)+ "\t0\n";
						    }
						else
						    eleEq = eleEq.substring(eleEq.indexOf('\n')+1);
					    }

				
				    }

				
				equivalentString.append(eleEq);
			    }
			else
			    System.out.println("Type not specified by element or type!!");
		    }
		else//specified by part type 
		    {
			int isStruct =0;

			if(isArrayType(getName(partType)))
			    {
				isStruct=1;
				partType =getName(partType);
			    }
			else if(isEnumType(partType))
			    {
				partType = getNS(partType,npartNode.getNameSpace())+getName(partType);
				isStruct=2;
			    }
			else if(isXsd(partType,xsdNS))
			    {
				isStruct = 0;
				partType = "xsd:"+getName(partType);
				if((getName(partType).compareToIgnoreCase("hexBinary")==0)||
				   (getName(partType).compareToIgnoreCase("base64binary")==0))
				    {
					//					partType = getName(partType);
					isStruct=1;
				    }
			    }
			else if(isSoapEnc(partType,encodingNS))
			    {
				partType = "soap-enc:"+getName(partType);
				if((getName(partType).compareToIgnoreCase("hexBinary")==0)||
				   (getName(partType).compareToIgnoreCase("base64binary")==0)){
				    //partType = getName(partType);
				isStruct=1;
				}
			    }
			else
			    {

				partType = getNS(partType,npartNode.getNameSpace())+getName(partType);
				isStruct=1;
			    }
			equivalentString.append(partType+ "\t"+isStruct+"\t" +uscore+convertToCpp(partName,true)+ "\t"+isStruct+"\n");
		    }

		npartNode = part.getNextNode();
		 if(npartNode!=null)
		     partNode = npartNode.getNode();
	    }

	return equivalentString.toString();
    }

    boolean isSoapEnc(String name,String currentSoapNS)
    {
	String typeName = getName(name);
	if(typeName.indexOf('[')!=-1)
	    typeName = typeName.substring(0,typeName.indexOf('['));
	
	if(currentSoapNS!=null)
	    if(getNS(name).compareToIgnoreCase(currentSoapNS)==0)
		{
		    allPrimitiveSOAPDataType.put(typeName,new Boolean(true));
		    return true;
		}

	if(getNS(name).compareToIgnoreCase("")==0)
	    {
		if(allPrimitiveSOAPDataType.containsKey(typeName))
		    {
			allPrimitiveSOAPDataType.put(typeName,new Boolean(true));
			return true;
		    }
	    }

	
	return false;
    }
    
    boolean isXsd(String name,String currentXSDNS)
    {
	String typeName = getName(name);
	if(typeName.indexOf('[')!=-1)
	    typeName = typeName.substring(0,typeName.indexOf('['));
	
	if((currentXSDNS!=null)&&(getNS(name).compareToIgnoreCase(currentXSDNS)==0))
	    {
		allPrimitiveXSDDataType.put(typeName,new Boolean(true));
		return true;
	    }
	
	if(getNS(name).compareToIgnoreCase("")==0)
	    {

		if(allPrimitiveXSDDataType.containsKey(typeName))
		    {
			allPrimitiveXSDDataType.put(typeName,new Boolean(true));
			return true;
		    }
	    }

	return false;
    }

    Vector keyWords = new Vector();

    private void intitlizePrimitiveType()
    {
        keyWords.add("auto");
	keyWords.add("bool");
	keyWords.add("break");
	keyWords.add("case");
	keyWords.add("char");
	keyWords.add("class");
	keyWords.add("const");
	keyWords.add("continue");
	keyWords.add("default");
	keyWords.add("do");
	keyWords.add("double");
	keyWords.add("else");
	keyWords.add("enum");
	keyWords.add("extern");
	keyWords.add("false");
	keyWords.add("float");
	keyWords.add("for");
	keyWords.add("goto");
	keyWords.add("if");
	keyWords.add("inline");
	keyWords.add("int");
	keyWords.add("long");
	keyWords.add("LONG64");
	keyWords.add("mustUnderstand");
	keyWords.add("namespace");
	keyWords.add("operator");
	keyWords.add("private");
	keyWords.add("protected");
	keyWords.add("public");
	keyWords.add("register");
	keyWords.add("return");
	keyWords.add("short");
	keyWords.add("signed");
	keyWords.add("sizeof");
	keyWords.add("static");
	keyWords.add("struct");
	keyWords.add("switch");
	keyWords.add("time_t");
	keyWords.add("true");
	keyWords.add("typedef");
	keyWords.add("ULONG64");
	keyWords.add("union");
	keyWords.add("unsigned");
	keyWords.add("using");
	keyWords.add("virtual");
	keyWords.add("void");
	keyWords.add("volatile");
	keyWords.add("wchar_t");
	keyWords.add("while");
	keyWords.add("restrict");



	allPrimitiveXSDDataType.put("anyType",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("anyType","typedef char * xsd__anyType;\n");
	allPrimitiveXSDDataType.put("anyURI",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("anyURI","typedef char * xsd__anyURI;\n");
	allPrimitiveXSDDataType.put("base64Binary",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("base64Binary","class xsd__base64Binary {\npublic:\nunsigned char *__ptr;\nint __size;\n};\n");
	allPrimitiveXSDDataType.put("boolean",new Boolean(false));

	if (outc)
	    allPrimitiveXSDDataTypeEquivalent.put("boolean","typedef enum { false_, true_ } xsd__boolean;\n");
	else
	    allPrimitiveXSDDataTypeEquivalent.put("boolean","typedef bool xsd__boolean;\n");

	allPrimitiveXSDDataType.put("byte",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("byte","typedef char xsd__byte;\n");
	allPrimitiveXSDDataType.put("dateTime",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("dateTime","typedef char * xsd__dateTime;\n");
	allPrimitiveXSDDataType.put("date",new Boolean(false));	
	allPrimitiveXSDDataTypeEquivalent.put("date","typedef char * xsd__date;\n");	
	allPrimitiveXSDDataType.put("decimal",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("decimal","typedef char * xsd__decimal;\n");
	allPrimitiveXSDDataType.put("double",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("double","typedef double xsd__double;\n");
	allPrimitiveXSDDataType.put("duration",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("duration","typedef char * xsd__duration;\n");
	allPrimitiveXSDDataType.put("float",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("float","typedef float xsd__float;\n");
	allPrimitiveXSDDataType.put("hexBinary",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("hexBinary","class xsd__hexBinary {\npublic:\nunsigned char *__ptr;\nint __size;\n};\n");
	allPrimitiveXSDDataType.put("int",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("int","typedef int xsd__int;\n");
	allPrimitiveXSDDataType.put("integer",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("integer","typedef char * xsd__nteger;\n");
	allPrimitiveXSDDataType.put("long",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("long","typedef LONG64 xsd__long;\n");
	allPrimitiveXSDDataType.put("negativeInteger",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("negativeInteger","typedef char * xsd__negativeInteger;\n");
	allPrimitiveXSDDataType.put("nonNegativeInteger",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("nonNegativeInteger","typedef char * xsd__nonNegativeInteger;\n");
	allPrimitiveXSDDataType.put("nonPositiveInteger",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("nonPositiveInteger","typedef char * xsd__nonPositiveInteger;\n");
	allPrimitiveXSDDataType.put("normalizedString",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("normalizedString","typedef char *  xsd__normalizedString;\n");
	allPrimitiveXSDDataType.put("positiveInteger",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("positiveInteger","typedef char * xsd__positiveInteger;\n");
	allPrimitiveXSDDataType.put("short",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("short","typedef char * xsd__short;\n");
	allPrimitiveXSDDataType.put("string",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("string","typedef char * xsd__string;\n");
	allPrimitiveXSDDataType.put("time",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("time","typedef char * xsd__time;\n");
	allPrimitiveXSDDataType.put("token",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("token","typedef char * xsd__token;\n");
	allPrimitiveXSDDataType.put("unsignedByte",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("unsignedByte","typedef unsigned char xsd__unsignedByte;\n");
	allPrimitiveXSDDataType.put("unsignedInt",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("unsignedInt","typedef unsigned int xsd__unsignedInt;\n");
	allPrimitiveXSDDataType.put("unsignedLong",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("unsignedLong","typedef unsigned LONG64 xsd__unsignedLong;\n");
	allPrimitiveXSDDataType.put("unsignedShort",new Boolean(false));
	allPrimitiveXSDDataTypeEquivalent.put("unsignedShort","typedef unsigned short xsd__unsignedShort;\n");




	allPrimitiveSOAPDataType.put("anyURI",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("anyURI","typedef char * soap_enc__anyURI;\n");
	allPrimitiveSOAPDataType.put("base64Binary",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("base64Binary","class soap_enc__base64Binary {\npublic:\nunsigned char *__ptr;\nint __size;\n};\n");
	allPrimitiveSOAPDataType.put("boolean",new Boolean(false));

	if (outc)
	    allPrimitiveSOAPDataTypeEquivalent.put("boolean","typedef enum { false_, true_ } soap_enc__boolean;\n");
	else
	    allPrimitiveSOAPDataTypeEquivalent.put("boolean","typedef bool soap_enc__boolean;\n");
	
	allPrimitiveSOAPDataType.put("byte",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("byte","typedef char soap_enc__byte;\n");
	allPrimitiveSOAPDataType.put("dateTime",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("dateTime","typedef char * soap_enc__dateTime;\n");
	allPrimitiveSOAPDataType.put("date",new Boolean(false));	
	allPrimitiveSOAPDataTypeEquivalent.put("date","typedef char * soap_enc__date;\n");	
	allPrimitiveSOAPDataType.put("decimal",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("decimal","typedef char * soap_enc__decimal;\n");
	allPrimitiveSOAPDataType.put("double",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("double","typedef double soap_enc__double;\n");
	allPrimitiveSOAPDataType.put("duration",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("duration","typedef char * soap_enc__duration;\n");
	allPrimitiveSOAPDataType.put("float",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("float","typedef float soap_enc__float;\n");
	allPrimitiveSOAPDataType.put("hexBinary",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("hexBinary","class soap_enc__hexBinary{\npublic:\nunsigned char *__ptr;\nint __size;\n};\n");
	allPrimitiveSOAPDataType.put("int",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("int","typedef int soap_enc__int;\n");
	allPrimitiveSOAPDataType.put("integer",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("integer","typedef char * soap_enc__integer;\n");
	allPrimitiveSOAPDataType.put("long",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("long","typedef LONG64 soap_enc__long;\n");
	allPrimitiveSOAPDataType.put("negativeInteger",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("negativeInteger","typedef char * soap_enc__negativeInteger;\n");
	allPrimitiveSOAPDataType.put("nonNegativeInteger",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("nonNegativeInteger","typedef char * soap_enc__nonNegativeInteger;\n");
	allPrimitiveSOAPDataType.put("nonPositiveInteger",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("nonPositiveInteger","typedef char * soap_enc__nonPositiveInteger;\n");
	allPrimitiveSOAPDataType.put("normalizedString",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("normalizedString","typedef char *  soap_enc__normalizedString;\n");
	allPrimitiveSOAPDataType.put("positiveInteger",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("positiveInteger","typedef char * soap_enc__positiveInteger;\n");
	allPrimitiveSOAPDataType.put("short",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("short","typedef char * soap_enc__short;\n");
	allPrimitiveSOAPDataType.put("string",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("string","typedef char * soap_enc__string;\n");
	allPrimitiveSOAPDataType.put("time",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("time","typedef char * soap_enc__time;\n");
	allPrimitiveSOAPDataType.put("token",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("token","typedef char * soap_enc__token;\n");
	allPrimitiveSOAPDataType.put("unsignedByte",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("unsignedByte","typedef unsigned char soap_enc__unsignedByte;\n");
	allPrimitiveSOAPDataType.put("unsignedInt",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("unsignedInt","typedef unsigned int soap_enc__unsignedInt;\n");
	allPrimitiveSOAPDataType.put("unsignedLong",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("unsignedLong","typedef unsigned LONG64 soap_enc__unsignedLong;\n");
	allPrimitiveSOAPDataType.put("unsignedShort",new Boolean(false));
	allPrimitiveSOAPDataTypeEquivalent.put("unsignedShort","typedef unsigned short soap_enc__unsignedShort;\n");

	if(outc)
	    {
		allPrimitiveXSDDataTypeEquivalent.put("base64Binary","struct xsd__base64Binary {\nunsigned char *__ptr;\nint __size;\n};\n");
		allPrimitiveXSDDataTypeEquivalent.put("hexBinary","struct xsd__hexBinary {\nunsigned char *__ptr;\nint __size;\n};\n");
		allPrimitiveSOAPDataTypeEquivalent.put("base64Binary","struct soap_enc__base64Binary {\nunsigned char *__ptr;\nint __size;\n};\n");
		allPrimitiveSOAPDataTypeEquivalent.put("hexBinary","struct soap_enc__hexBinary{\nunsigned char *__ptr;\nint __size;\n};\n");
	    }


    }	


	
    public void getTypesInformation(Node node)
    {
	NsNodeSearch types = new NsNodeSearch(node,docNS+"types",new Hashtable());

	NsNode ntypesNode = types.getNextNode();


	if(ntypesNode==null)
	    {
		if(defWsdl){
		    types = new NsNodeSearch(node,"types",new Hashtable());
		    ntypesNode = types.getNextNode();
		}
		    
		if(ntypesNode==null){
		System.out.println(" Types node not found");
		return;}
	    }

	Node typesNode = ntypesNode.getNode();
	
	NsNodeSearch schema = new NsNodeSearch(typesNode,"schema",ntypesNode.getNameSpace());
	NsNode schemaNode = schema.getNextPartialMatch();
	System.out.println("Number Of Schemas in Document is "+schema.getTotalMatches());

	for(int i=0; i<schema.getTotalMatches();i++)
	    {

		Node cnode = schemaNode.getNode();
		/*find a element node just below the schema node*/
		String dataName = null;
		NodeList cnodes = cnode.getChildNodes();
		String schemaNS = cnode.getNodeName().substring(0,cnode.getNodeName().indexOf("schema"));

		boolean processed=false;

		for (int j=0; j<cnodes.getLength(); j++){

		    //process element nodefor doc literal
		    if(cnodes.item(j).getNodeName().equalsIgnoreCase(schemaNS+"element"))
			{
			    dataName = getAttrValue(cnodes.item(j),"name");

			    NsNode celementNode = new NsNode(cnodes.item(j),schemaNode.getNameSpace());

			    processSchema(celementNode,dataName,schemaNS);
			    processed=true;
			}

		    //process for rpc encoding
		    else if(cnodes.item(j).getNodeName().equalsIgnoreCase(schemaNS+"complexType"))
			{
			    NsNode cComplexNode = new NsNode(cnodes.item(j),schemaNode.getNameSpace());
			    processSchema(cComplexNode,null,schemaNS);
			    processed = true;
			}
		    //process for rpc encoding
		    else if(cnodes.item(j).getNodeName().equalsIgnoreCase(schemaNS+"simpleType"))
			{
			    NsNode cComplexNode = new NsNode(cnodes.item(j),schemaNode.getNameSpace());
			    processSchema(cComplexNode,null,schemaNS);
			    processed = true;
			}

		}
		/*end find a element node just below the schema node*/

		if (!processed){
		    processSchema(schemaNode,null,schemaNS);
		}

		schemaNode = schema.getNextPartialMatch();
	    }
	processAllStructures();

	processAllElements();
    }

    void processSchema(NsNode nnode,String dataName,String schNS)
    {
	String localXSDNS=getNsEquivalent(nnode.getNameSpace(),"xsdNS");

	String tns=getNsEquivalent(nnode.getNameSpace(),"tns");
	String currentNS=getNsEquivalent(nnode.getNameSpace(),"definitionNS");
	String schemaNS =null;
	String localencodingNS=getNsEquivalent(nnode.getNameSpace(),"encodingNS");
	String wsdlNS = getNsEquivalent(nnode.getNameSpace(),"wsdlNS");
	Hashtable referenceNameSpace=null;
	Node node=null;
	
	if(nnode!=null)
	    node = nnode.getNode();

	if(schNS==null)
	    {
		String nodeName = node.getNodeName();
		schemaNS = nodeName.substring(0,nodeName.indexOf("schema"));
	    }
	else
	    schemaNS = schNS;




	
	NsNodeSearch simpleType = new NsNodeSearch(node,schemaNS+"simpleType",nnode.getNameSpace());

	NsNode nsimpleNode = simpleType.getNextNode();
	
	Node simpleNode=null;
	if(nsimpleNode!=null)
	    simpleNode = nsimpleNode.getNode();

	referenceNameSpace=null;

	for(int i=0;i<simpleType.getTotalMatches();i++)
	    {
		String enumName = currentNS+getAttrValue(simpleNode,"name");

		if(enumName!=null)
		    {
			
			String enumEquivalent="";
			int dataType = 2;

			
			Hashtable attrib = new Hashtable();
			//attrib.put("base", localXSDNS+"string");
			attrib.put("base", localXSDNS+"string");
			
			NsNodeSearch restriction = new
			    NsNodeSearch(simpleNode,schemaNS+"restriction",attrib,nsimpleNode.getNameSpace());
			//NsNodeSearch(simpleNode,schemaNS+"restriction",attrib,nsimpleNode.getNameSpace());
			NsNode nrestrictionNode = restriction.getNextNode();
			
			NsNodeSearch enumeration = new
			    NsNodeSearch(simpleNode,schemaNS+"enumeration",nsimpleNode.getNameSpace());
			NsNode nenumerationNode = enumeration.getNextNode();
			
			NsNodeSearch list = new
			    NsNodeSearch(simpleNode,schemaNS+"list",nsimpleNode.getNameSpace());
			NsNode nlistNode = list.getNextNode();
			
		
			//case for enum and enum mask.
			if((nrestrictionNode!=null)&&(nenumerationNode!=null))
			    {
				if(nlistNode!=null)
				    dataType = 1;
				
				//enumEquivalent =dataType("enum "+maskStr+currentNS+getAttrValue(simpleNode,"name"))+" { ";
				for(int j=0;j<enumeration.getTotalMatches();j++)
				    {
					enumEquivalent
					    +=convertToCpp(getAttrValue(nenumerationNode.getNode(),"value"),true)+", ";
					nenumerationNode = enumeration.getNextNode();
				    }
				if(enumEquivalent.lastIndexOf(',')!=-1)
				    enumEquivalent = enumEquivalent.substring(0,enumEquivalent.lastIndexOf(','));
				
				enumName = getNS(enumName,nnode.getNameSpace())+getName(enumName);
				addDataType(enumName,dataType,enumEquivalent);
				processedEnums.add(enumName);
			    }
			else 
			    {
				restriction = new NsNodeSearch(simpleNode,schemaNS+"restriction",nsimpleNode.getNameSpace());
				//NsNodeSearch(simpleNode,schemaNS+"restriction",attrib,nsimpleNode.getNameSpace());
				nrestrictionNode = restriction.getNextNode();
				
				String description;
				
				if(nrestrictionNode != null)
				    {
					String enumType = getAttrValue(nrestrictionNode.getNode(),"base");

					if(isXsd(enumType,localXSDNS))
					    {
						enumType ="xsd:"+getName(enumType);
						
						description = enumType+"\t"+0+"\t_var1\t"+0+"\n";
						processedTypeDefs.add(enumName);
						addDataType(enumName,6,description);
					    }
					else if(isSoapEnc(enumType,localencodingNS))
					    {
						enumType = "soap-enc:"+getName(enumType);
						
						description = enumType+ "\t"+0+"\t_var1\t"+0+"\n";
						processedTypeDefs.add(enumName);
						addDataType(enumName,6,description);
					    }
					
					else
					    {
						enumType = getNS(enumType,nrestrictionNode.getNameSpace())+getName(enumType);
						
						if(isTypeDef(enumType))
						    {
							enumType =  getTypeDefEquivalent(enumType);
							description = enumType+ "\t"+0+"\t_var1\t"+0+"\n";
							processedTypeDefs.add(enumName);
							addDataType(enumName,6,description);
						    }
						else
						    {
							description = (String)allDataType.get(enumType);
							if(description!=null)
							    allDataType.put(enumName,description);
						    }
					    }
				    }
			    }
			
			nsimpleNode = simpleType.getNextNode();
			if(nsimpleNode!=null)
			    simpleNode = nsimpleNode.getNode();
		    }
	    }




	NsNodeSearch complexType = new NsNodeSearch(node,schemaNS+"complexType",nnode.getNameSpace());

	NsNode ncomplexNode = complexType.getNextNode();
	
	Node complexNode=null;

	
	if(ncomplexNode!=null)
	    complexNode = ncomplexNode.getNode();

	


	for(int i=0;i<complexType.getTotalMatches();i++)
	    //	if(complexType.getTotalMatches()!=0)
	    {
		String arrayType,arrayName;
		/*processing for a Array identified by restricition*/

		
		if((new NodeSearch(complexNode,schemaNS+"restriction").getNextNode())!=null)
		    {

			if(dataName==null)
			    arrayName = getAttrValue(complexNode,"name");
			else 
			    arrayName = getName(dataName);
				
			Hashtable attributesref = new Hashtable();
			attributesref.put("ref",localencodingNS+"Array");
			
			NsNodeSearch attribute  = new NsNodeSearch(complexNode,schemaNS+"attribute",ncomplexNode.getNameSpace());
			NsNode nattributeNode = attribute.getNextNode();
			Node attributeNode=null;
			if(nattributeNode!=null)
			    {
				attributeNode = nattributeNode.getNode();
				referenceNameSpace = nattributeNode.getNameSpace();
				wsdlNS = getNsEquivalent(nattributeNode.getNameSpace(),"wsdlNS");
			    }
			
			if(attributeNode == null)
			    {
				//this case occurs in some docs where the
				//typeof array is specified using attribute
				// element qualified by a arraytype

				NsNodeSearch element = new NsNodeSearch(complexNode,schemaNS+"element",nnode.getNameSpace());
				NsNode nelementNode = element.getNextNode();
				Node elementNode =null;
				
				if(nelementNode!=null)
				    {
					elementNode = nelementNode.getNode();
					referenceNameSpace = nelementNode.getNameSpace();
				    }

				if(elementNode==null)
				    {
					System.out.println("even element Node not found");
					return;
				    }
				arrayType = getAttrValue(elementNode,"type");
				arrayType = arrayType+"[]";
				//assumption that the datatype is a one dimension array of the give type 
			    }
			else
			    {
				arrayType =getAttrValue(attributeNode,wsdlNS+"arrayType");
			    }

			if(arrayType==null)
			    {
				System.out.println("No Type attribute found for element "+arrayName);
				ncomplexNode = complexType.getNextNode();
				if(ncomplexNode!=null)
				    complexNode = ncomplexNode.getNode();

				return;
				//continue;
			    }
			

			String curlocalXSDNS = getNsEquivalent(referenceNameSpace,"xsdNS");
			String curlocalSoapNS = getNsEquivalent(referenceNameSpace,"soapNS");
			String curlocalencodingNS = getNsEquivalent(referenceNameSpace,"encodingNS");
			int isStruct =0;

			/*if(getNS(arrayType).compareToIgnoreCase(curlocalXSDNS)==0)*/

			if(isXsd(arrayType,curlocalXSDNS))
			    {
				arrayType = "xsd:"+getName(arrayType);
				if((getName(arrayType).compareToIgnoreCase("hexBinary")==0)||
				   (getName(arrayType).compareToIgnoreCase("base64binary")==0)){
				    //arrayType = getName(arrayType);
				    isStruct=1;
				}
			    }
			else if(isSoapEnc(arrayType,curlocalencodingNS))
			    {
				arrayType = "soap-enc:"+getName(arrayType);
				if((getName(arrayType).compareToIgnoreCase("hexBinary")==0)||
				   (getName(arrayType).compareToIgnoreCase("base64binary")==0)){
				    // arrayType = getName(arrayType);
				    isStruct=1;
				}
			    }
			else
			    {
				arrayType = getNS(arrayType,nnode.getNameSpace())+getName(arrayType);
				isStruct = 1;
			    }


			if(singleLevelArray(arrayType))
			    {
				String description;
				int dim = getArrayDimension(arrayType);

				String type = arrayType.substring(0,arrayType.indexOf('['));
				
				if(dim==1)
				    {
					//description = "struct "+dataType(type)+"\t*__ptr;\nint\t__size;\nint\t__offset;\n};";
					description=type+"\t1\t__ptr\t"+isStruct+"\n"+
					    "int\t0\t__size\t3\n"+
					    "int\t0\t__offset\t3\n";
				    }
				else
				    {
					description=type+"\t1\t__ptr\t"+isStruct+"\n"+
					    "int\t0\t__size["+dim+"]\t3\n"+
					    "int\t0\t__offset["+dim+"]\t3\n";
				    }

				addDataType(arrayName,4,description);
				processedArrays.put(arrayType,arrayName);
				allArrays.add(arrayName);
			    }
			else
			    {
				/*add in to  TOBE process que*/
				tobeProcessedArrays.put(arrayType,arrayName+"\n"+curlocalXSDNS+"\n"+curlocalencodingNS);
			    }
		    }
		else
		    {

			if(dataName==null)
			    dataName = "";

			structureNodes.put(ncomplexNode,dataName+"\n"+schemaNS);

			if(dataName.equals(""))
			    dataName = null;

			
		    }
		ncomplexNode = complexType.getNextNode();
		if(ncomplexNode!=null)
		    complexNode = ncomplexNode.getNode();
	    }

	/*System.out.println("to be processed que is "+tobeProcessedArrays.size());*/
	if((simpleType.getTotalMatches()==0)&&(complexType.getTotalMatches()==0))
	    {
		elementNodes.put(nnode,schemaNS);
	    }

		
	processPendingArrays();

    }


    private void processAllElements()
    {
	Enumeration keyList;
	keyList = elementNodes.keys();

	while(keyList.hasMoreElements())
	    {
		NsNode nelementNode = (NsNode)keyList.nextElement();
		
		String currentNS = getNsEquivalent(nelementNode.getNameSpace(),"definitionNS");		
		String dataTypeName  =getAttrValue(nelementNode.getNode(),"name");
		String type = getAttrValue(nelementNode.getNode(),"type");

		String curlocalXSDNS =  getNsEquivalent(nelementNode.getNameSpace(),"xsdNS");
		String curlocalencodingNS = getNsEquivalent(nelementNode.getNameSpace(),"encodingNS");


		if((type!=null)&&(dataTypeName!=null))
		    {
			dataTypeName = currentNS+dataTypeName;
			dataTypeName = getNS(dataTypeName,nelementNode.getNameSpace())+getName(dataTypeName);

			type = getNS(type,nelementNode.getNameSpace())+getName(type);


			String description=null;
			int isStruct = 0 ;
			
			if(isXsd(type,curlocalXSDNS))
			    {
				type ="xsd:"+getName(type);
				
				//special condition for binary
				if((getName(type).compareToIgnoreCase("hexBinary")==0)||
				   (getName(type).compareToIgnoreCase("base64Binary")==0))
				    {
					//type = getName(type);
					isStruct=1;
				    }

				description = type+"\t"+isStruct+"\t_var1\t"+isStruct+"\n";
				processedTypeDefs.add(dataTypeName);
				addDataType(dataTypeName,6,description);
			    }
			else if(isSoapEnc(type,curlocalencodingNS))
			    {
				type = "soap-enc:"+getName(type);
				if((getName(type).compareToIgnoreCase("hexBinary")==0)||
				   (getName(type).compareToIgnoreCase("base64Binary")==0))
				    {
					//type = getName(type);
					isStruct=1;
				    }
				description = type+ "\t"+isStruct+"\t_var1\t"+isStruct+"\n";
				processedTypeDefs.add(dataTypeName);
				addDataType(dataTypeName,6,description);
			    }
			
			else
			    {
				description = (String)allDataType.get(type);
				if(description!=null)
				    allDataType.put(dataTypeName,description);
			    }
			
		    }
	    }
	/*need to add code for unprocessed elements looping !!!!*/
    }

    
    private void processAllStructures()
    {

	Hashtable unProcessedStructures = new Hashtable();

	Enumeration keyList;

	keyList = structureNodes.keys();
	boolean baseClassNotYetProcessed = false;
	boolean dataTypeAddedInThisIteration = false;
	
	while(keyList.hasMoreElements())
	    {
		NsNode ncomplexNode = (NsNode)keyList.nextElement();
		String datasch = (String)structureNodes.get(ncomplexNode);
		String dataName = datasch.substring(0,datasch.indexOf('\n'));
		String schemaNS = datasch.substring(datasch.indexOf('\n')+1);
		/*get it from hashtable ... put it along with dataname*/
		Node complexNode=ncomplexNode.getNode();
		String currentNS=getNsEquivalent(ncomplexNode.getNameSpace(),"definitionNS");
		/*String schemaNS =null;*/
		Hashtable referenceNameSpace=null;
		String description = "";

		String structName =null;

		String occursString = "__size";

		int structType = 3;
		
		baseClassNotYetProcessed = false;
		
		if(dataName.equals(""))
		    structName= getAttrValue(complexNode,"name");
		else
		    structName = dataName;
		
	        //processing for a strucuture

		/*this doesnt work if the extension calss is processed before
		 * base class */
		
		NsNodeSearch extension = new NsNodeSearch(complexNode,schemaNS+"extension",ncomplexNode.getNameSpace());
		NsNode nextensionNode = extension.getNextNode();

		String base= null;
		int lineCount=0;
		boolean derived = false;
		
		while(nextensionNode!=null)//the data type is extension of some other type
		    {
			derived = true;
			
			Node extensionNode = nextensionNode.getNode();

			String baseName = getAttrValue(extensionNode,"base");

			baseName = getNS(baseName,nextensionNode.getNameSpace())+getName(baseName);
				
			if(base==null)
			    base = baseName;
			else
			    base += ", "+baseName;
			
			String dataTypeEquivalent = (String)allDataType.get(baseName);
			
			if(dataTypeEquivalent!=null)
			    {
				lineCount += getLineCount(dataTypeEquivalent);
				description +=dataTypeEquivalent.substring(dataTypeEquivalent.indexOf('\n')+1);
			    }
			else
			    baseClassNotYetProcessed = true;
			
			nextensionNode = extension.getNextNode();
		    }

		if(baseClassNotYetProcessed)
		    {
			unProcessedStructures.put(ncomplexNode,datasch);
			continue;
		    }
		else if (derived)
		    {
			structType = 5;
			description= base + "\t"+(lineCount-1)+"\n"+description;
		    }
		
		NsNodeSearch element = new NsNodeSearch(complexNode,schemaNS+"element",ncomplexNode.getNameSpace());
		
		NsNode nelementNode = element.getNextNode();
		
		Node elementNode =null;
		
		if(nelementNode!=null)
		    {
			elementNode =nelementNode.getNode();
			referenceNameSpace = nelementNode.getNameSpace();
		    }

		boolean processingError = false;
		
		for(int j=0;j<element.getTotalMatches()&&(!processingError);j++)
		    {
			String typeStr = getAttrValue(elementNode,"type");

			if(typeStr !=null)
			    typeStr = getNS(typeStr,nelementNode.getNameSpace())+getName(typeStr); 
			
			String nameStr = getAttrValue(elementNode,"name");
			
			String curlocalXSDNS =  getNsEquivalent(referenceNameSpace,"xsdNS");
			String curlocalSoapNS = getNsEquivalent(referenceNameSpace,"soapNS");
			String curlocalencodingNS = getNsEquivalent(referenceNameSpace,"encodingNS");
			
			String ptr=" ";
			
			boolean maxoccursFlag = false;
			
			String nillable = getAttrValue(elementNode,"nillable");

			int isStructure=0;
			
			if((nillable!=null)&&(nillable.compareToIgnoreCase("true")==0))
			    ptr="*";

			if(typeStr==null)
			    {
				j++;
				nelementNode = element.getNextNode();

				if(nelementNode!=null)
				    {
					elementNode = nelementNode.getNode();
					referenceNameSpace = nelementNode.getNameSpace();
				    }
				
				String ref = getAttrValue(elementNode,"ref");
				
				if(ref!=null)
				    System.out.println("Warning : The strucutre "+structName + " uses ref which is not currently supported by the WSDL Importer\n\t The generated header file may be incomplete\n");
				else
				    {
					System.out.println("Warning : The elements of Strucutre " +structName+" could not be found\n\t The generated header file may be incomplete\n");
					continue;
				    }
			    }
			else if(isArrayType(getName(typeStr)))
			    {
				isStructure = 1;
				typeStr =getName(typeStr);
			    }
			else if(isEnumType(typeStr))
			    {
				typeStr =getNS(typeStr)+getName(typeStr);
				isStructure = 2;
			    }
			else if(isXsd(typeStr,curlocalXSDNS))
			    {
				isStructure = 0;
				typeStr = "xsd:"+getName(typeStr);
				//special condition for hexBinary and base 64Binary
				
				if((getName(typeStr).compareToIgnoreCase("hexBinary")==0)||(getName(typeStr).compareToIgnoreCase("base64Binary")==0))
					isStructure = 1;
			    }
			
			else if(isSoapEnc(typeStr,curlocalencodingNS))
			    {
				isStructure = 0;
				typeStr = "soap-enc:"+getName(typeStr);
				//special condition for hexBinary and base 64Binary
				
				if((getName(typeStr).compareToIgnoreCase("hexBinary")==0)||(getName(typeStr).compareToIgnoreCase("base64Binary")==0))
				    isStructure = 1;
			    }
			else
			    {
				typeStr = getNS(typeStr)+getName(typeStr);
				isStructure = 1;
			    }

			if(isStructure == 1) ptr = "*";
			
			String maxOccurs = getAttrValue(elementNode,"maxOccurs");

			if((maxOccurs!=null)&&(maxOccurs.equals("unbounded")))
			    {
				ptr = "*";
				occursString  +="_";
				maxoccursFlag = true;
			    }

			/*max occurs is unbound*/
			if(maxoccursFlag)
			    description+="int\t0\t"+occursString+"\t3\n";

			int isPtr=0;
			
			if(ptr.equals("*"))
			    isPtr=1;
			
			description+=typeStr+"\t"+isPtr+"\t"+convertToCpp(nameStr,true)+"\t"+isStructure+"\n";

			nelementNode = element.getNextNode();
			
			if(nelementNode!=null)
			    {
				elementNode = nelementNode.getNode();
				referenceNameSpace = nelementNode.getNameSpace();
			    }
		    }
		

		structName = getNS(currentNS+structName,ncomplexNode.getNameSpace())+getName(currentNS+structName);

		addDataType(structName,structType,description);
		dataTypeAddedInThisIteration = true;
	    }
	if(!unProcessedStructures.isEmpty())
	    {
		if(dataTypeAddedInThisIteration)
		    {
			structureNodes = unProcessedStructures;
			processAllStructures();
		    }
		else
		    {
			System.out.println("Some Structures are not generated as corresponding base classes were not found");
		    }
	    }
    }

    int getLineCount(String str)
    {
	int count=0;
	
	for(int i=0;i<str.length();i++)
	    if(str.charAt(i)=='\n')
		count++;
	return count;
    }

    boolean isArrayType(String name)
    {
	if(name==null)
	{
	    System.out.println("Error: Name of dataType is found to be NULL. This may be a bug in WSDL importer");
	    System.exit(0);
	}
	
	if(allArrays.contains(getName(name)))
	    return true;
	return false;
    }
    
    boolean isTypeDef(String name)
    {
	if(name==null)
	    {
		System.out.println("Error: Name of dataType is found to be NULL. This may be a bug in WSDL importer");
		System.exit(0);
	    }
	//	System.out.println(processedTypeDef);
	
	if(processedTypeDefs.contains(name))
	   return true;

	  return false;
	    
    }

    String getTypeDefEquivalent(String name)
    {
	if(allDataType.contains(name))
	    {
		String description;

		description= (String)allDataType.get(name);
		
		StringTokenizer lineToken = new StringTokenizer(description,"\n");
		
		int dataType = Integer.parseInt((String)lineToken.nextElement());
		
		StringTokenizer tabToken = new StringTokenizer((String)lineToken.nextElement(),"\t");
		String dType =(String)tabToken.nextElement();
		return dType;
	    }
	else
	    return new String("char *");
    }
    
    

    boolean isEnumType(String name)
    {
	if(name==null)
	{
	    System.out.println("Error: Name of dataType is found to be NULL. This may be a bug in WSDL importer");
	    System.exit(0);
	}

	if(processedEnums.contains(name))
	    return true;
	return false;
    }
    
    private void processPendingArrays()
    {
	Enumeration keyList;

	boolean someProcessed = false;
	
	keyList = tobeProcessedArrays.keys();
	
	while(keyList.hasMoreElements())
	    {
		String arrType = (String)keyList.nextElement();
		

		String temp= (String)tobeProcessedArrays.get(arrType);

		String arrName=temp.substring(0,temp.indexOf('\n'));
		
		String curlocalXSDNS = temp.substring(temp.indexOf('\n')+1);
		String curlocalencodingNS = temp.substring(temp.indexOf('\n')+1,temp.lastIndexOf('\n')+1);
		String baseType = arrType.substring(0,arrType.lastIndexOf("["));

		if(processedArrays.containsKey(baseType))
		    {
			String description;
			
			someProcessed = true;

			String baseName = (String)processedArrays.get(baseType);
			
			int dim = getArrayDimension(arrType);

			String arrayType = baseName;



			/*always the array of arrays elements will be a
			 * structure as teh base type will have to be
			 * structure(array of something)*/

			if(dim==1)
			    {
				description=arrayType+"\t1\t__ptr\t1\n"+
				    "int\t0\t__size\t3\n"+
				    "int\t0\t__offset\t3\n";
			    }
			else
			    {
				description=arrayType+"\t1\t__ptr\t1\n"+
				    "int\t0\t__size["+dim+"]\t3\n"+
				    "int\t0\t__offset["+dim+"]\t3\n";
			    }

			addDataType(arrName,4,description);
			
			processedArrays.put(arrType,arrName);
			allArrays.add(arrName);
			tobeProcessedArrays.remove(arrType);
		    }
	    }
	
	if(someProcessed)
	    {
		if(tobeProcessedArrays.size()!=0)
		    {
			processPendingArrays();
		    }
	    }
	else
	    {
		if(tobeProcessedArrays.size()!=0)
		    System.out.println("Warning : some array element left unprocessed and the size of to be processed array is "+tobeProcessedArrays.size());
	    }  
    }

    
    Hashtable allDataType = new Hashtable();
    String addDataType(String name, int type,String description)
    {
	description=type+"\n"+description;
	//	System.out.println("name = "+name+"\n"+"description = "+description+"\ttype:"+ type);
	
	boolean inserted = false;
	while(!inserted)
	    {
		if(!allDataType.containsKey(name))
		    {

			allDataType.put(name,description);
			/*			if(type.equals("enum"))
			    allDataType.put(name,type+" "+dataType(cNS+name)+" { "+description);
			else
			    allDataType.put(name,type+" "+dataType(cNS+name)+" {\n"+description);
			*/
			//System.out.println(name+"  "+type+" "+dataType(cNS+name)+" {\n"+description);
			inserted = true;
		    }
		else
		    {
			if(((String)allDataType.get(name)).compareToIgnoreCase(description)==0)
			    return name;
			name = name+"-";
		    }
	    }
	return name;
    }
    
    private int getArrayDimension(String atype)
    {
	int lindex = atype.lastIndexOf("[");
	String last = atype.substring(lindex,atype.length());

	int cnt=0;

	int indx=-1;

	while((indx=last.indexOf(',',indx+1))!=-1)cnt++;

	return cnt+1;
    }

    private int getArrayLevel(String atype)
    {
	int cnt=0;

	int indx=-1;
	while((indx=atype.indexOf('[',indx+1))!=-1)cnt++;

	return cnt;

    }
    
    
    private boolean singleLevelArray(String atype)
    {
	int cnt=0;

	int indx=-1;
	while((indx=atype.indexOf('[',indx+1))!=-1)cnt++;
	
	if(cnt==1)
	    return true;
	else
	    return false;
    }

    private String getNS(String name)
    {
        if(name.indexOf(':')==-1)
            return "";
        return name.substring(0,name.indexOf(':')+1);
    }

    
    Hashtable globalNameSpaces = new Hashtable();
    int lcnt=0;
    private String getNS(String name,Hashtable localHashTable)
    {
	lcnt++;
	if(name.indexOf(':')==-1)
	    return "";

	String nameSpace =name.substring(0,name.indexOf(':'));

	
	//System.out.println(" ns is "+ nameSpace);
	String uri = (String)localHashTable.get("xmlns:"+nameSpace);

	if(uri==null)
	    {
		try{
		System.out.println("ERROR : the name space "+nameSpace+" "+lcnt+" not found in the local Hash Table "+localHashTable);
		throw new Exception("");

		}catch(Exception e){e.printStackTrace();System.exit(0);}
	    }

	String curGlobalNameSpaceForURI = (String)globalNameSpaces.get(uri);
	
	if(curGlobalNameSpaceForURI!=null)
	    {
		return curGlobalNameSpaceForURI.substring(curGlobalNameSpaceForURI.indexOf(':')+1)+":";
		
	    }
	else
	    {
		int ctr=1;
		while(globalNameSpaces.containsValue("xmlns:"+nameSpace))nameSpace+=ctr;
		globalNameSpaces.put(uri,"xmlns:"+nameSpace);
		return nameSpace+":";
	    }
    }
    
    private String getName(String name)
    {
	if(name.indexOf(':')==-1)
	    return name;
	return name.substring(name.indexOf(':')+1);
    }	
	      

    public void parseDoc(Node node)
    {

	intitlizePrimitiveType();
	
	if(node.getNodeType()!=Node.DOCUMENT_NODE)
	    {
		System.out.println("Expected a Document node!!!!!");
		System.exit(0);
	    }
	NodeList nodes = node.getChildNodes();
	//	node = nodes.item(0);	/* go to a lever below definitions*/
	/*nodes = node.getChildNodes();*/


	if (nodes != null)
	    {
		
		setNameSpaces(node);

		getTypesInformation(node);

		getServiceInfo(node);
		getBindingInfo(node,servicePortBinding.substring(servicePortBinding.indexOf(":")+1));
	    }
    }
    
    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: java wsdlcpp [XML URI]");
            System.exit(0);
        }
	
        String uri =null;
	boolean cFlag=false;
	for(int i=0;i<args.length;i++)
	    {
		if(args[i].trim().toLowerCase().endsWith("wsdl"))
		    uri=args[i];
		if(args[i].trim().toLowerCase().equals("-c"))
		    cFlag = true;
	    }
        wsdlcpp parserParser = new wsdlcpp();
	parserParser.gsoapParser(uri,cFlag);
    }
}

