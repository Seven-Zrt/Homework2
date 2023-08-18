#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include"parser.h"
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#define N1 400
void Transmat(double a[][30], double Jac[][30], double b[30], double re[30], int num) {//将矩阵中的数值转存
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < num; j++) {
            a[i][j] = Jac[i+1][j+1];
        }
        b[i] = -re[i + 1];
    }
}
double stripString(char* stringIn) {
    char buf[BufLength], buf2[BufLength];
    int a, b;
    strcpy(buf, stringIn);
    for (a = 0; buf[a] != '='; a++) {};
    a++;
    for (b = 0; buf[a] != '\0'; b++, a++)
        buf2[b] = buf[a];
    buf2[b] = '\0';
    return atof(buf2);
};
void multiple(double A[][30],double b[],double c[],int number) {//进行矩阵运算
    for (int i = 0; i < number; i++) {
        double sum = 0.0;
        for (int j = 0; j < number; j++) {
            sum += A[i][j] * b[j];
        }
        c[i] += sum;
    }
}
bool isright(double a[], double acc, int number) {
    for (int i = 0; i < number; i++) {
        if (abs(a[i]) > acc)
            return false;
    }
    return true;
}
char* strComponentType(Component* compPtr) {

    char* compTypeName = new char[6];
    switch (compPtr->getType()) {

    case VSource: strcpy(compTypeName, "V"); break;
    case Resistor: strcpy(compTypeName, "R"); break;
    case BJT: strcpy(compTypeName, "T"); break;
    case MOSFET: strcpy(compTypeName, "M"); break;
    case ISource: strcpy(compTypeName, "I"); break;
    case Inductor: strcpy(compTypeName, "ind"); break;
    case Diode: strcpy(compTypeName, "Diode"); break;
    case Capacitor: strcpy(compTypeName, "Cap"); break;
    }

    return compTypeName;
}


void printComponents(Component* compPtr) {
    char compTypeName[6];
    cout << endl << "Components: " << endl << endl;
    while (compPtr != NULL) {
        strcpy(compTypeName, strComponentType(compPtr));
        cout << "->" << compTypeName << compPtr->getcompNum();
        compPtr = compPtr->getNext();
    }
    cout << endl;
    return;
}

void printNodes(Node* nodePtr, int compFlag) {

    Connections* conPtr;
    cout << endl << "Nodes: " << endl << endl;
    while (nodePtr != NULL) {
        if (compFlag == 0) { //It is printed just the names of the nodes
            cout << "-> " << nodePtr->getNameNum();
        }
        else if (compFlag == 1) { //It is printed the nodes and the connections
            cout << "-> " << nodePtr->getNameNum() << " {";
            conPtr = nodePtr->getConList();
            while (conPtr->next != NULL) {
                cout << strComponentType(conPtr->comp) << conPtr->comp->getcompNum() << ", ";
                conPtr = conPtr->next;
            }
            cout << strComponentType(conPtr->comp) << conPtr->comp->getcompNum() << '}' << endl;
        }
        else {
            cout << "Invalid value for compFlag. (0) to print just nodes, (1) to print nodes and connections!";
            exit(1);

        }

        nodePtr = nodePtr->getNext();
    }


    return;
}
int main() {
    ifstream inFile("C:/Users/12520/Desktop/netlist.txt");
    ofstream outFile("C:/Users/12520/Desktop/out.txt");
    char inName[NameLength], outName[NameLength], buf[BufLength],
        buf1[BufLength], buf2[BufLength], buf3[BufLength], nameBuf[NameLength],
        * bufPtr, * charPtr1, * charPtr2;
    int intBuf1, intBuf2, intBuf3, intBuf4, datum = 0, eqNum = NA, specPrintJacMNA = 0;
    double douBuf1=0.0, douBuf2=0.0, douBuf3=0.0, douBuf4=0.0;
    CompType typeBuf;
    Component* compPtr, * compPtr1, * compPtr2;
    Node* nodePtr, * nodePtr1, * nodePtr2;
    Model* modelPtr;
    TranType TtypeBuf;
    EquaType eqType = Modified;
    DcType dctype = Nr;
    NodeHead nodeList;
    CompHead compList;
    ModelHead modelList;
    if (eqNum == NA) {
        while ((eqNum != 1) && (eqNum != 2)) {
            cout << "Available Equations Types Are:" << endl
                << " <1>  Nodal" << endl
                << " <2>  Modified Nodal" << endl
                << "Please enter your choice <1, 2>:" << endl;
            cin >> buf;
            eqNum = atoi(buf);
        }
        if (eqNum == 1)
            eqType = Nodal;
        else if (eqNum == 2)
            eqType = Modified;
    }
    printf("please input dctype:\n1.N-R\n2.Homotopy\n");
    printf("please input your choice:\n");
    //选择直流分析的方法
    int dcnum = 0;
    scanf("%d", &dcnum);
    if (dcnum == 1) dctype = Nr;
    else if (dcnum == 2) dctype = Homotopy;
    // parsing of netlist to create linked list of models (remember to reset the fstream)头几行
    inFile.getline(buf, BufLength);       // first line of netlist is discarded
    inFile.getline(buf, BufLength);

    while (inFile.good()) {
        if ((buf == NULL) || (*buf == '\0')) {
            inFile.getline(buf, BufLength);
            continue;
        }
        strcpy(buf1, buf);
        if (!strcmp(strtok(buf1, " "), ".model")) {
            strcpy(buf2, strtok(NULL, " "));
            charPtr1 = strtok(NULL, " ");
            if (!strcmp(charPtr1, "PNP"))
                TtypeBuf = PNP;
            else if (!strcmp(charPtr1, "NPN"))
                TtypeBuf = NPN;
            else if (!strcmp(charPtr1, "NMOS"))
                TtypeBuf = NMOS;
            else if (!strcmp(charPtr1, "PMOS"))
                TtypeBuf = PMOS;

            charPtr1 = strtok(NULL, " ");
            while (charPtr1 != NULL) {
                strcpy(buf3, "");
                if ((charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=')) {
                    douBuf1 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'B') && (charPtr1[1] == 'F') && (charPtr1[2] == '=')) {
                    douBuf2 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'B') && (charPtr1[1] == 'R') && (charPtr1[2] == '=')) {
                    douBuf3 = stripString(charPtr1);
                }
                if ((charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[2] == '=')) {
                    douBuf4 = stripString(charPtr1);
                }
                charPtr1 = strtok(NULL, " ");
            }
            modelPtr = new Model(buf2, TtypeBuf, douBuf1, douBuf2, douBuf3, douBuf4);
            modelList.addModel(modelPtr);
        }
        inFile.getline(buf, BufLength);
    }
    inFile.close();
    inFile.open("C:/Users/12520/Desktop/netlist.txt", ios::in);
    char model_str[9];
    inFile.getline(buf, BufLength);       // first line of netlist is discarded
    inFile.getline(buf, BufLength);
    while (inFile.good()) {//初始化
        if ((buf == NULL) || (*buf == '\0')) {
            inFile.getline(buf, BufLength);
            continue;
        }

        if (isalpha(*buf)) {

            //  EDIT THIS SECTION IF NEW COMPONENTS ARE ADDED!!!
            //  we could do some rearranging in this section to catch each type in order.
            switch (*buf) {
            case 'v':
            case 'V':
                typeBuf = VSource;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            case 'i':
            case 'I':
                cout << "I" << endl;
                typeBuf = ISource;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            case 'q':
            case 'Q':
                typeBuf = BJT;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                intBuf3 = atoi(strtok(NULL, " "));
                compPtr = new Component(typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, NA,
                    modelList.getModel(strtok(NULL, " ")), nameBuf);
                compList.addComp(compPtr);
                break;
            case 'm':
            case 'M':
                typeBuf = MOSFET;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                intBuf3 = atoi(strtok(NULL, " "));
                intBuf4 = atoi(strtok(NULL, " "));
                compPtr = new Component(typeBuf, NA, NA, intBuf1, intBuf2, intBuf3, intBuf4,
                    modelList.getModel(strtok(NULL, " ")), nameBuf);
                compList.addComp(compPtr);
                break;
            case 'r':
            case 'R':
                typeBuf = Resistor;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            case 'd':
            case 'D':
                typeBuf = Diode;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                charPtr1 = strtok(NULL, " ");
                while (charPtr1 != NULL) {
                    if ((charPtr1[0] == 'I') && (charPtr1[1] == 'S') && (charPtr1[2] == '=')) {
                        douBuf1 = stripString(charPtr1);
                    }
                    if ((charPtr1[0] == 'T') && (charPtr1[1] == 'E') && (charPtr1[4] == '=')) {
                        douBuf2 = stripString(charPtr1);
                    }
                    charPtr1 = strtok(NULL, " ");
                }
                compPtr = new Component(typeBuf, douBuf1, douBuf2, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            case 'c':
            case 'C':
                typeBuf = Capacitor;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            case 'l':
            case 'L':
                typeBuf = Inductor;
                strcpy(nameBuf, strtok(buf, " "));
                intBuf1 = atoi(strtok(NULL, " "));
                intBuf2 = atoi(strtok(NULL, " "));
                douBuf1 = atof(strtok(NULL, " "));
                compPtr = new Component(typeBuf, douBuf1, NA, intBuf1, intBuf2, NA, NA, NULL, nameBuf);
                compList.addComp(compPtr);
                break;
            };
        }
        inFile.getline(buf, BufLength);
    }
    compPtr1 = compList.getComp(0);
    while (compPtr1 != NULL) {//组件连接节点
        for (int b = 0; b < 3; b++) {
            if ((!compPtr1->isCon(b)) && (compPtr1->getConVal(b) != NA)) { //1.判断是否已经连接2.判断是否有效
                intBuf1 = compPtr1->getConVal(b); //返回b端口所连接的节点号
                nodePtr1 = nodeList.addNode();
                nodePtr1->setNameNum(intBuf1);  
                compPtr1->connect(b, nodePtr1); 
                nodePtr1->connect(b, compPtr1); 

                
                compPtr2 = compPtr1->getNext();
                while (compPtr2 != NULL) {
                    for (int c = 0; c < 3; c++) { 
                        if (compPtr2->getConVal(c) == intBuf1) { 
                            compPtr2->connect(c, nodePtr1);
                            nodePtr1->connect(c, compPtr2);
                            break;                                    
                        }
                    }
                    compPtr2 = compPtr2->getNext();
                }
            }
        }
        compPtr1 = compPtr1->getNext();
    }
    //证明输入datum是否有效
    Boolean check = FALSE;
    if (datum != NA) {
        nodePtr = nodeList.getNode(0);
        while (nodePtr != NULL) {
            if (nodePtr->getNameNum() == datum)
                check = TRUE;
            nodePtr = nodePtr->getNext();
        }
        if (check == FALSE) {
            cerr << "Datum value invalid!" << endl
                << "PROGRAM EXITED ABNORMALLY!" << endl;
            exit(0);
        }
    }

    // Loop to find lastnode
    nodePtr = nodeList.getNode(0); //~> getting the pointer to the first node, pointed by 'headNode'
    int lastnode = nodePtr->getNameNum();
    while (nodePtr != NULL) {
        lastnode = (nodePtr->getNameNum() > lastnode) ? nodePtr->getNameNum() : lastnode;
        nodePtr = nodePtr->getNext();
    }

    //  Loop to find the datum
    if (datum == NA) {//找连接器件数最多的节点
        nodePtr = nodeList.getNode(0);
        nodePtr1 = nodePtr->getNext();
        while (nodePtr1 != NULL) {
            if (nodePtr1->getCount() > nodePtr->getCount())
                nodePtr = nodePtr1;
            nodePtr1 = nodePtr1->getNext();
        }
        datum = nodePtr->getNameNum();
    }
  //真正开始建立方程
    nodePtr = nodeList.getNode(0);
    while (nodePtr != NULL) {
        if (nodePtr->getNameNum() != datum) {
            nodePtr->printNodal(outFile, datum, lastnode);
        }
        nodePtr = nodePtr->getNext();
    }
    compPtr = compList.getComp(0);
    while (compPtr != NULL) {
        compPtr->specialPrint(outFile, datum);
        compPtr = compPtr->getNext();
    }
    if (eqType != Modified) {
        compPtr = compList.getComp(0);
        while (compPtr != NULL) {
            compPtr->printSuperNode(outFile, datum, lastnode);
            compPtr = compPtr->getNext();
        }
    }
    if (eqType == Modified) {
        nodePtr = nodeList.getNode(0);
        while (nodePtr != NULL) {
            if (nodePtr->getNameNum() != datum)
                nodePtr->printMNA(outFile, datum, lastnode);
            nodePtr = nodePtr->getNext();
        }
    }
    //雅可比矩阵
    outFile << endl
        << "%*****************************************************************************" << endl;
    outFile << endl << "%                      Jacobians: " << endl;
    nodePtr1 = nodeList.getNode(0);
    while (nodePtr1 != NULL) {   //~> this loop handles the nodes not connected to a Vsource and those ones that are not the 'datum' node
        if (nodePtr1->getNameNum() != datum) {//行
            nodePtr2 = nodeList.getNode(0);
            while (nodePtr2 != NULL) {//列
                if (nodePtr2->getNameNum() != datum) {
                    nodePtr1->printJac(outFile, datum, nodePtr2, lastnode, eqType);
                }
                nodePtr2 = nodePtr2->getNext();
            }
        }
        nodePtr1 = nodePtr1->getNext();
    }
    // go down the component list and give equations for all sources
    compPtr = compList.getComp(0);
    while (compPtr != NULL) {
        nodePtr2 = nodeList.getNode(0);
        compPtr2 = compList.getComp(0);
        while (nodePtr2 != NULL) {
            if (nodePtr2->getNameNum() != datum) {
                compPtr->specialPrintJac(outFile, datum, nodePtr2/**/, lastnode, eqType, compPtr2, &specPrintJacMNA /**/); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
            }
            nodePtr2 = nodePtr2->getNext();
        }
        specPrintJacMNA = 0;
        compPtr = compPtr->getNext();
    }

    // print the Jacobians for the additional MNA equations改进节点法
    if (eqType == Modified) {
        nodePtr1 = nodeList.getNode(0);
        while (nodePtr1 != NULL) {
            if (nodePtr1->getNameNum() != datum) {
                nodePtr2 = nodeList.getNode(0);
                while (nodePtr2 != NULL) {
                    if (nodePtr2->getNameNum() != datum)
                        nodePtr1->printJacMNA(outFile, datum, nodePtr2, lastnode);
                    nodePtr2 = nodePtr2->getNext();
                }
            }
            nodePtr1 = nodePtr1->getNext();
        }
    }




    //进行NR迭代
    if (dctype == Nr) {
        int number = 0;//方程维数
        int count = 1;//迭代次数
        double accruacy;
        printf("Please input the dimension of the equation:\n ");
        scanf("%d", &number);
        printf("Please input the initial solution of the equation:\n");
        for (int i = 0; i < number; i++) {
            scanf("%lf", &nodeValue[i + 1]);
        }
        printf("Please input the expected accuracy:\n");
        scanf("%lf", &accruacy);
        //遍历整条电路得到第一次迭代前的矩阵和值
        nodePtr = nodeList.getNode(0);
        while (nodePtr != NULL) {
            if (nodePtr->getNameNum() != datum) {
                nodePtr->printNodalMat(datum, lastnode, result);
            }
            nodePtr = nodePtr->getNext();
        }

        compPtr = compList.getComp(0);
        while (compPtr != NULL) {
            compPtr->specialPrintMat(datum, result);
            compPtr = compPtr->getNext();
        }


        //~> go down the component list and give supernode equations for all float sources (Nodal Analysis)
        if (eqType != Modified) {
            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                compPtr->printSuperNodeMat(datum, lastnode, result);
                compPtr = compPtr->getNext();
            }
        }


        // go down the node list and give additional MNA equations
        if (eqType == Modified) {
            nodePtr = nodeList.getNode(0);
            while (nodePtr != NULL) {
                if (nodePtr->getNameNum() != datum)
                    nodePtr->printMNAMat(datum, lastnode, result);
                nodePtr = nodePtr->getNext();
            }
        }





        nodePtr1 = nodeList.getNode(0);
        while (nodePtr1 != NULL) {
            if (nodePtr1->getNameNum() != datum) {
                nodePtr2 = nodeList.getNode(0);
                while (nodePtr2 != NULL) {
                    if (nodePtr2->getNameNum() != datum) {
                        nodePtr1->printJacMat(datum, nodePtr2, lastnode, eqType, jacMat);
                    }
                    nodePtr2 = nodePtr2->getNext();
                }
            }
            nodePtr1 = nodePtr1->getNext();
        }

        // go down the component list and give equations for all sources
        compPtr = compList.getComp(0);
        while (compPtr != NULL) {
            nodePtr2 = nodeList.getNode(0);
            compPtr2 = compList.getComp(0);
            while (nodePtr2 != NULL) {
                if (nodePtr2->getNameNum() != datum) {
                    compPtr->specialPrintJacMat(datum, nodePtr2, lastnode, eqType, compPtr2, &specPrintJacMNA, jacMat); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
                }
                nodePtr2 = nodePtr2->getNext();
            }
            specPrintJacMNA = 0;
            compPtr = compPtr->getNext();
        }




        // print the Jacobians for the additional MNA equations
        if (eqType == Modified) {
            nodePtr1 = nodeList.getNode(0);
            while (nodePtr1 != NULL) {
                if (nodePtr1->getNameNum() != datum) {
                    nodePtr2 = nodeList.getNode(0);
                    while (nodePtr2 != NULL) {
                        if (nodePtr2->getNameNum() != datum)
                            nodePtr1->printJacMNAMat(datum, nodePtr2, lastnode, jacMat);
                        nodePtr2 = nodePtr2->getNext();
                    }
                }
                nodePtr1 = nodePtr1->getNext();
            }
        }
        double A[30][30] = { 0.0 };
        double B[30][30] = { 0.0 };//用于存储逆矩阵
        double b[30] = { 0.0 };
        double diff[30] = { 0.0 };
        Transmat(A, jacMat, b, result, number);
        Gauss(A, B, number);//系数矩阵求逆存入B中
        multiple(B, b, diff, number);//矩阵乘法
        for (int i = 0; i < number; i++) {
            nodeValue[i + 1] = nodeValue[i + 1] + diff[i];//迭代一次
        }


        while (!isright(diff, accruacy, number)) {//开始迭代过程
            count++;
            for (int i = 0; i < number; i++) {//重置矩阵
                for (int j = 0; j < number; j++) {
                    jacMat[i + 1][j + 1] = 0.0;
                }
                result[i + 1] = 0;
            }
            nodePtr = nodeList.getNode(0);
            while (nodePtr != NULL) {
                if (nodePtr->getNameNum() != datum) {
                    nodePtr->printNodalMat(datum, lastnode, result);
                }
                nodePtr = nodePtr->getNext();
            }

            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                compPtr->specialPrintMat(datum, result);
                compPtr = compPtr->getNext();
            }


            //~> go down the component list and give supernode equations for all float sources (Nodal Analysis)
            if (eqType != Modified) {
                compPtr = compList.getComp(0);
                while (compPtr != NULL) {
                    compPtr->printSuperNodeMat(datum, lastnode, result);
                    compPtr = compPtr->getNext();
                }
            }


            // go down the node list and give additional MNA equations
            if (eqType == Modified) {
                nodePtr = nodeList.getNode(0);
                while (nodePtr != NULL) {
                    if (nodePtr->getNameNum() != datum)
                        nodePtr->printMNAMat(datum, lastnode, result);
                    nodePtr = nodePtr->getNext();
                }
            }





            nodePtr1 = nodeList.getNode(0);
            while (nodePtr1 != NULL) {
                if (nodePtr1->getNameNum() != datum) {
                    nodePtr2 = nodeList.getNode(0);
                    while (nodePtr2 != NULL) {
                        if (nodePtr2->getNameNum() != datum) {
                            nodePtr1->printJacMat(datum, nodePtr2, lastnode, eqType, jacMat);
                        }
                        nodePtr2 = nodePtr2->getNext();
                    }
                }
                nodePtr1 = nodePtr1->getNext();
            }

            // go down the component list and give equations for all sources
            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                nodePtr2 = nodeList.getNode(0);
                compPtr2 = compList.getComp(0);
                while (nodePtr2 != NULL) {
                    if (nodePtr2->getNameNum() != datum) {
                        compPtr->specialPrintJacMat(datum, nodePtr2, lastnode, eqType, compPtr2, &specPrintJacMNA, jacMat); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
                    }
                    nodePtr2 = nodePtr2->getNext();
                }
                specPrintJacMNA = 0;
                compPtr = compPtr->getNext();
            }




            // print the Jacobians for the additional MNA equations
            if (eqType == Modified) {
                nodePtr1 = nodeList.getNode(0);
                while (nodePtr1 != NULL) {
                    if (nodePtr1->getNameNum() != datum) {
                        nodePtr2 = nodeList.getNode(0);
                        while (nodePtr2 != NULL) {
                            if (nodePtr2->getNameNum() != datum)
                                nodePtr1->printJacMNAMat(datum, nodePtr2, lastnode, jacMat);
                            nodePtr2 = nodePtr2->getNext();
                        }
                    }
                    nodePtr1 = nodePtr1->getNext();
                }
            }
            Transmat(A, jacMat, b, result, number);
            Gauss(A, B, number);//系数矩阵求逆存入B中
            multiple(B, b, diff, number);//矩阵乘法
            for (int i = 0; i < number; i++) {
                nodeValue[i + 1] = nodeValue[i + 1] + diff[i];//迭代一次
            }
        }
        cout << count << endl << endl;
        cout << "----------------------------------------------------------------------------------" << endl;
        for (int i = 0; i < number; i++) {
            cout << nodeValue[i + 1] << endl;
        }
    }
    



    //-------------------------------------------------同伦法求解
    if (dctype == Homotopy) {//同伦法
        int number = 0;//方程维数
        printf("Please input the dimension of the equation:\n ");
        scanf("%d", &number);
        printf("Please input the initial solution of the equation:\n");
        for (int i = 0; i < number; i++) {
            scanf("%lf", &nodeValue[i + 1]);
        }
        double step = 100;//选定步长
        double result_zero[30] = { 0.0 };//用于存放未迭代过的初值result
        printf("please input step:\n");
        scanf("%lf", &step);


        //遍历整条电路得到第一次迭代前的矩阵和值
        nodePtr = nodeList.getNode(0);
        while (nodePtr != NULL) {
            if (nodePtr->getNameNum() != datum) {
                nodePtr->printNodalMat(datum, lastnode, result);
            }
            nodePtr = nodePtr->getNext();
        }

        compPtr = compList.getComp(0);
        while (compPtr != NULL) {
            compPtr->specialPrintMat(datum, result);
            compPtr = compPtr->getNext();
        }


        //~> go down the component list and give supernode equations for all float sources (Nodal Analysis)
        if (eqType != Modified) {
            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                compPtr->printSuperNodeMat(datum, lastnode, result);
                compPtr = compPtr->getNext();
            }
        }


        // go down the node list and give additional MNA equations
        if (eqType == Modified) {
            nodePtr = nodeList.getNode(0);
            while (nodePtr != NULL) {
                if (nodePtr->getNameNum() != datum)
                    nodePtr->printMNAMat(datum, lastnode, result);
                nodePtr = nodePtr->getNext();
            }
        }





        nodePtr1 = nodeList.getNode(0);
        while (nodePtr1 != NULL) {
            if (nodePtr1->getNameNum() != datum) {
                nodePtr2 = nodeList.getNode(0);
                while (nodePtr2 != NULL) {
                    if (nodePtr2->getNameNum() != datum) {
                        nodePtr1->printJacMat(datum, nodePtr2, lastnode, eqType, jacMat);
                    }
                    nodePtr2 = nodePtr2->getNext();
                }
            }
            nodePtr1 = nodePtr1->getNext();
        }

        // go down the component list and give equations for all sources
        compPtr = compList.getComp(0);
        while (compPtr != NULL) {
            nodePtr2 = nodeList.getNode(0);
            compPtr2 = compList.getComp(0);
            while (nodePtr2 != NULL) {
                if (nodePtr2->getNameNum() != datum) {
                    compPtr->specialPrintJacMat(datum, nodePtr2, lastnode, eqType, compPtr2, &specPrintJacMNA, jacMat); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
                }
                nodePtr2 = nodePtr2->getNext();
            }
            specPrintJacMNA = 0;
            compPtr = compPtr->getNext();
        }




        // print the Jacobians for the additional MNA equations
        if (eqType == Modified) {
            nodePtr1 = nodeList.getNode(0);
            while (nodePtr1 != NULL) {
                if (nodePtr1->getNameNum() != datum) {
                    nodePtr2 = nodeList.getNode(0);
                    while (nodePtr2 != NULL) {
                        if (nodePtr2->getNameNum() != datum)
                            nodePtr1->printJacMNAMat(datum, nodePtr2, lastnode, jacMat);
                        nodePtr2 = nodePtr2->getNext();
                    }
                }
                nodePtr1 = nodePtr1->getNext();
            }
        }
        //对初值result赋值
        for (int i = 0; i < number; i++) {
            result_zero[i] = -result[i + 1];
        }




        double k = 1.0;
        double A[30][30] = { 0.0 };//用于代存雅可比矩阵
        double B[30][30] = { 0.0 };//用于存储逆矩阵
        double b[30] = { 0.0 };
        double diff[30] = { 0.0 };
        while (k != step) {
            for (int i = 0; i < number; i++) {//重置矩阵
                for (int j = 0; j < number; j++) {
                    jacMat[i + 1][j + 1] = 0.0;
                }
                result[i + 1] = 0;
            }
            nodePtr = nodeList.getNode(0);
            while (nodePtr != NULL) {
                if (nodePtr->getNameNum() != datum) {
                    nodePtr->printNodalMat(datum, lastnode, result);
                }
                nodePtr = nodePtr->getNext();
            }

            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                compPtr->specialPrintMat(datum, result);
                compPtr = compPtr->getNext();
            }


            //~> go down the component list and give supernode equations for all float sources (Nodal Analysis)
            if (eqType != Modified) {
                compPtr = compList.getComp(0);
                while (compPtr != NULL) {
                    compPtr->printSuperNodeMat(datum, lastnode, result);
                    compPtr = compPtr->getNext();
                }
            }


            // go down the node list and give additional MNA equations
            if (eqType == Modified) {
                nodePtr = nodeList.getNode(0);
                while (nodePtr != NULL) {
                    if (nodePtr->getNameNum() != datum)
                        nodePtr->printMNAMat(datum, lastnode, result);
                    nodePtr = nodePtr->getNext();
                }
            }





            nodePtr1 = nodeList.getNode(0);
            while (nodePtr1 != NULL) {
                if (nodePtr1->getNameNum() != datum) {
                    nodePtr2 = nodeList.getNode(0);
                    while (nodePtr2 != NULL) {
                        if (nodePtr2->getNameNum() != datum) {
                            nodePtr1->printJacMat(datum, nodePtr2, lastnode, eqType, jacMat);
                        }
                        nodePtr2 = nodePtr2->getNext();
                    }
                }
                nodePtr1 = nodePtr1->getNext();
            }

            // go down the component list and give equations for all sources
            compPtr = compList.getComp(0);
            while (compPtr != NULL) {
                nodePtr2 = nodeList.getNode(0);
                compPtr2 = compList.getComp(0);
                while (nodePtr2 != NULL) {
                    if (nodePtr2->getNameNum() != datum) {
                        compPtr->specialPrintJacMat(datum, nodePtr2, lastnode, eqType, compPtr2, &specPrintJacMNA, jacMat); // ~> specPrintJacMNA is used to verify if the jacobians w.r.t. the Modified equations was already printed to print only once.
                    }
                    nodePtr2 = nodePtr2->getNext();
                }
                specPrintJacMNA = 0;
                compPtr = compPtr->getNext();
            }




            // print the Jacobians for the additional MNA equations
            if (eqType == Modified) {
                nodePtr1 = nodeList.getNode(0);
                while (nodePtr1 != NULL) {
                    if (nodePtr1->getNameNum() != datum) {
                        nodePtr2 = nodeList.getNode(0);
                        while (nodePtr2 != NULL) {
                            if (nodePtr2->getNameNum() != datum)
                                nodePtr1->printJacMNAMat(datum, nodePtr2, lastnode, jacMat);
                            nodePtr2 = nodePtr2->getNext();
                        }
                    }
                    nodePtr1 = nodePtr1->getNext();
                }
            }
            double temp[30] = { 0.0 };//临时数组，用于存放构造函数的结果
            Transmat(A, jacMat, b, result, number);
            for (int i = 0; i < number; i++) {
                temp[i] = b[i] + (k / step - 1) * result_zero[i];
            }
            Gauss(A, B, number);
            multiple(B, temp, diff, number);
            for (int i = 0; i < number; i++) {
                nodeValue[i + 1] = nodeValue[i + 1] + diff[i];
            }
            k+=1.0;
        }
        for (int i = 0; i < number; i++) {
            cout << nodeValue[i + 1] << endl;
        }
    }
  



    //瞬态分析
    double E = 0.0, R = 0.0, C = 0.0;
    double U = 0.0;
    compPtr = compList.getComp(0);
    while (compPtr != NULL) {
        if (compPtr->getType() == VSource) {
            E = compPtr->getVal();
        }
        else if (compPtr->getType() == Resistor) {
            R = compPtr->getVal();
        }
        else if (compPtr->getType() == Capacitor) {
            C = compPtr->getVal();
        }
        compPtr = compPtr->getNext();
    }
    FILE* output = fopen("C:/Users/12520/Desktop/data.csv", "w");
    double h = 0.001, T = 0.4;
    double h_0 = 0.0;
    while (h_0 < 0.4) {
        U = (h * E) / (R * C) + (1 - h / (R * C)) * U;
        fprintf(output, "%f\n", U);
        h_0 += h;
    }
    fclose(output);
    system("python plot.py");
	return 0;
}
