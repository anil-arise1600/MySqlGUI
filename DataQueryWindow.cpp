/**
* @file DataQueryWindow.cpp
* @author Anil Kumar
* @date 15-11-2017
* @brief This DataQueryWindow class, it's handle all SQL query as per clicked button.
*/

#include "DataQueryWindow.h"
#include "ui_DataQueryWindow.h"

/**
 * @brief DataQueryWindow::DataQueryWindow
 * @param parent
 * @param db_
 */
DataQueryWindow::DataQueryWindow(QWidget *parent, QSqlDatabase *db_) :
    QMainWindow(parent),
    ui(new Ui::DataQueryWindow)
{
    ui->setupUi(this);
    dba = db_;
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    connect(ui->treeView,SIGNAL(clicked(QModelIndex)),this,SLOT(On_Click_treeview(QModelIndex)));

    connect(ui->actionChange_Database,SIGNAL(triggered(bool)),SLOT(on_pushButton_changeDB_clicked()));
    connect(ui->actionQuit,SIGNAL(triggered(bool)),SLOT(sl_ExitClose()));
    connect(ui->actionAbout,SIGNAL(triggered(bool)),SLOT(sl_About()));

    ui->lineEdit_Query->installEventFilter(this);

    ui->tableView_QueryData->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    sl_TreeViewList();
    ui->pushButton_Upload->hide();
    connect(ui->pushButton_Exit,SIGNAL(clicked()),this,SLOT(sl_ExitClose()));
    connect(ui->pushButton_Clear,SIGNAL(clicked()),ui->lineEdit_Query,SLOT(clear()));
    ui->pushButton_Exit->setDefault(true);

    isExport=false;

}

/**
 * @brief DataQueryWindow::~DataQueryWindow
 */
DataQueryWindow::~DataQueryWindow()
{
    delete ui;
    delete Item;
    delete ItemMain;
    delete Model;
}

/**
 * @brief DataQueryWindow::sl_TreeViewList
 */
void DataQueryWindow::sl_TreeViewList()
{
    ui->lineEdit_Query->clear();
    int i=0;
    PRINT(__FUNCTION__)<<"in tree Database "<<dba->databaseName();
    ui->label_selectedDB->setText(dba->databaseName());
    QSqlQuery query0("use "+dba->databaseName());
    query0.next();

    Model = new QStandardItemModel;
    ItemMain= new QStandardItem(dba->databaseName()+"@"+dba->hostName());
    Model->setHorizontalHeaderItem(0,ItemMain);
    ItemMain= new QStandardItem(dba->databaseName());
    Model->setItem(0,ItemMain);

    QSqlQuery query1("show tables from "+dba->databaseName());
    while(query1.next())
    {
        Item = new QStandardItem (query1.value(0).toString());
        ItemMain->appendRow(Item);
        i++;
    }
    if(i==0)
        ui->pushButton_Upload->show();
    else
        ui->pushButton_Upload->hide();
    ui->treeView->setModel(Model);
    ui->treeView->expandAll();
    ui->treeView->show();
}

/**
 * @brief DataQueryWindow::on_pushButton_Go_clicked
 */
void DataQueryWindow::on_pushButton_Go_clicked()
{
    ui->tableView_QueryData->repaint();

    ui->label_error->clear();
    strlist.clear();
    if(!ui->lineEdit_Query->text().isEmpty())
    {
        str_query=ui->lineEdit_Query->text();
        QSqlQuery Test_query;
        bool test_bool= Test_query.exec(str_query);
        if(test_bool)
        {
            int i=0;
            PRINT(__FUNCTION__)<<str_query<<str_query.length();
            int SpaceCount=0;
            while(i<str_query.length()-1)
            {
                if(str_query[i]==' ')
                    SpaceCount++;
                i++;
            }

            i=0;
            while(i<SpaceCount+1)
            {
                strlist<<str_query.split(" ").at(i);
                i++;
            }

            //PRINT(__FUNCTION__)<<"list"<<strlist;
            i=0;
            if(strlist.at(0)=="select" && strlist.at(1)=="*")
            {
                if(strlist.size()>4)
                    sl_SelectAll(strlist.at(3),strlist);
                else
                    sl_SelectAll(strlist.at(3),strlist);
            }
            else if(strlist.at(0)=="select" && strlist.at(1)!="*")
            {
                int size;int col=1;
                for(int k=0;k<strlist.size();k++)
                {
                    QString strr=strlist.at(k);
                    if(strr.toLower()=="from")
                        size=k-1;
                }

                Model = new QStandardItemModel;
                while(size>0)
                {
                    QString str1=strlist.at(col);
                    //PRINT(__FUNCTION__)<<str1<<str1[str1.size()-1];
                    QString str2;
                    if(str1[str1.size()-1]==',')
                    {
                        int j=0;
                        while(j<str1.size()-1)
                        {
                            str2[j]=str1[j];
                            j++;
                        }
                        //PRINT(__FUNCTION__)<<"coma-word "<<str2;
                    }
                    else
                        str2=strlist.at(col);
                    //PRINT(__FUNCTION__)<<"str2 "<<str2;
                    Item = new QStandardItem (str2);
                    Model->setHorizontalHeaderItem(i,Item);
                    i++;
                    col++;
                    size--;
                }
                int totalColumn=i;
                i=0;
                QSqlQuery query2;
                bool ex = query2.exec(str_query);
                if(ex)
                {
                    while(query2.next())
                    {

                        for(int k=0;k<totalColumn;k++)
                        {
                            Item = new QStandardItem (query2.value(k).toString());
                            Model->setItem(i,k,Item);

                            ////////// * alternate color of rows  */////////////////
                            if(i%2==0)
                                Model->item(i,k)->setBackground(Qt::darkGray);
                            else
                                Model->item(i,k)->setBackground(Qt::lightGray);
                            ////////////////////////////////////////////////////////
                        }
                        i++;
                    }
                    ui->label_error->setText("<font color=green>Displayed  !!</font>");
                }
                else
                    ui->label_error->setText(query2.lastError().text());
                ui->tableView_QueryData->setModel(Model);
            }
            else
            {
                QString str=strlist.at(0);
                PRINT(__FUNCTION__)<<"keyword "<<str.toLower();
                if(str.toLower()=="update"||str.toLower()=="drop"||str.toLower()=="create"||str.toLower()=="delete"||str.toLower()=="insert")
                {
                    QSqlQuery query2;
                    bool ex = query2.exec(str_query);
                    if(!ex)
                    {
                        ui->label_error->setText(query2.lastError().text().split(";").at(0));
                        PRINT(__FUNCTION__)<<query2.lastError().text().split(";").at(0);
                    }
                    else
                    {
                        QStringList tmpList;
                        if(str.toLower()=="update")
                        {
                            ui->label_error->setText("<font color=green>Successfully updated !!</font>");
                            sl_SelectAll(strlist.at(1),tmpList);
                        }
                        else if(str.toLower()=="drop")
                        {
                            ui->label_error->setText("<font color=green>Successfully droped !!</font>");
                            //sl_SelectAll(strlist.at(1));
                            sl_TreeViewList();
                        }
                        else if(str.toLower()=="create" && strlist.size()==3)
                        {
                            ui->label_error->setText("<font color=green>Successfully created database !!</font>");
                            //sl_SelectAll(strlist.at(1));
                            dba->setDatabaseName(strlist.at(2));
                            sl_TreeViewList();
                        }
                        else if(str.toLower()=="delete")
                        {
                            ui->label_error->setText("<font color=green>deleted !!</font>");
                            sl_SelectAll(strlist.at(2),tmpList);
                        }
                        else if(str.toLower()=="create" && QString(strlist.at(1)).toLower()=="table")
                        {
                            sl_TreeViewList();
                            ui->tableView_QueryData->setModel(nullptr);
                        }
                        else if(str.toLower()=="insert")
                        {
                            sl_SelectAll(strlist.at(2),tmpList);
                        }
                    }
                }
            }
        }
        else
            ui->label_error->setText(Test_query.lastError().text());
    }
    else
        PRINT(__FUNCTION__)<<"Query not found !";

}

/**
 * @brief DataQueryWindow::On_Click_treeview
 * @param M_index
 */
void DataQueryWindow::On_Click_treeview(QModelIndex M_index)
{
    QVariant data = ui->treeView->model()->data(M_index);
    QString text = data.toString();
    //PRINT(__FUNCTION__)<< "Model Index "/*<<M_index */<<text;
    if(text!=dba->databaseName())
    {
        QStringList tmpList;
        ui->lineEdit_Query->setText("select * from "+text);
        sl_SelectAll(text,tmpList);
    }
    else
    {
        ui->tableView_QueryData->setModel(nullptr);
        ui->lineEdit_Query->clear();
    }

}

/**
 * @brief DataQueryWindow::sl_ExitClose
 */
void DataQueryWindow::sl_ExitClose()
{
    emit si_ShowLogin();
//    dba->close();
    this->close();
}

void DataQueryWindow::sl_SelectAll(QString tableName, QStringList strListQuery)
{
    ui->label_Table->setText(tableName);
    int i=0;

    Model =new QStandardItemModel;

    QSqlQuery query4("show columns from "+tableName);
    while(query4.next())
    {
        Item = new QStandardItem (query4.value(0).toString());
        Model->setHorizontalHeaderItem(i,Item);
        i++;
    }
    int totalColumn=i;
    QString strQuery;
    if(strListQuery.size()>4)
    {
        foreach (QString str, strListQuery)
            strQuery += str+" ";
    }
    else
        strQuery = "select * from "+tableName;

    QSqlQuery query3(strQuery.trimmed());
    i=0;
    while (query3.next())
    {
        for(int k=0;k<totalColumn;k++)
        {
            Item = new QStandardItem (query3.value(k).toString());
            Model->setItem(i,k,Item);

            ////////// * alternate color of rows  */////////////////
            if(i%2==0)
                Model->item(i,k)->setBackground(Qt::darkGray);
            else
                Model->item(i,k)->setBackground(Qt::lightGray);
            ////////////////////////////////////////////////////////
        }
        i++;
    }
    ui->tableView_QueryData->setModel(Model);

    if(i>1 && strListQuery.size()>4)
        ui->label_error->setText("<font color=green>Successfully fetch data  !!</font>");
}

/**
 * @brief DataQueryWindow::on_pushButton_changeDB_clicked
 */
void DataQueryWindow::on_pushButton_changeDB_clicked()
{
    if(obj_sdb)
        delete obj_sdb;

    obj_sdb = new DialogSwitchDB(this,dba);
    connect(obj_sdb,SIGNAL(si_ClosedSwitchDB(bool)),SLOT(sl_ClosedSwitchDB(bool)));
    obj_sdb->exec();

}

void DataQueryWindow::on_pushButton_Upload_clicked()
{
    //PRINT(__FUNCTION__)<<dba->databaseName()<<dba->userName()<<dba->password();
    DialogUploadSQL *Upload_sql = new DialogUploadSQL(0,isExport,dba);
    connect(Upload_sql,SIGNAL(refresh()),this,SLOT(sl_TreeViewList()));
    Upload_sql->show();
}

/**
 * @brief DataQueryWindow::onCustomContextMenu
 * @param point
 */
void DataQueryWindow::onCustomContextMenu(const QPoint &point)
{
    PRINT(__FUNCTION__)<<"right click";
    Mdl_Index = ui->treeView->indexAt(point);
    Table_Name.clear();
    Table_Name= ui->treeView->model()->data(Mdl_Index).toString();
    PRINT(__FUNCTION__)<<Mdl_Index<<Table_Name;

    QMenu *menu = new QMenu(this);
    if(dba->databaseName() == Table_Name)
    {
        QAction *import= new QAction(QIcon(":/images/add.png"),tr("Import"),this);
        connect(import, &QAction::triggered, this, &DataQueryWindow::sl_Import_table);
        menu->addAction(import);

        QAction *_export= new QAction(QIcon(":/images/bottom.png"),tr("Export"),this);
        connect(_export, &QAction::triggered, this, &DataQueryWindow::sl_Export_table);
        menu->addAction(_export);
    }
    else
    {
        QAction *insert= new QAction(QIcon(":/images/add.png"),tr("Insert values"),this);
        connect(insert, &QAction::triggered, this, &DataQueryWindow::sl_Insert_table_values);
        menu->addAction(insert);

        QAction *browse= new QAction(QIcon(":/images/clear.png"),tr("Browse"),this);
        connect(browse, &QAction::triggered, this, &DataQueryWindow::sl_Browse_table);
        menu->addAction(browse);

        QAction *structure= new QAction(QIcon(":/images/settings.png"),tr("Structure"),this);
        connect(structure, &QAction::triggered, this, &DataQueryWindow::sl_Struct_table);
        menu->addAction(structure);

        QAction *sl_Drop_table= new QAction(QIcon(":/images/delete.png"),tr("Drop table"),this);
        connect(sl_Drop_table, &QAction::triggered, this, &DataQueryWindow::sl_Drop_table);
        menu->addAction(sl_Drop_table);
    }
    menu->popup(ui->treeView->viewport()->mapToGlobal(point));

}

/**
 * @brief DataQueryWindow::sl_Insert_table_values
 */
void DataQueryWindow::sl_Insert_table_values()
{
    this->setDisabled(true);
    PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name<<Mdl_Index;
    if(ivalue)
        delete ivalue;

    ivalue = new InsertValue(0,Table_Name);
    connect(ivalue,SIGNAL(si_RefreshTable(QString,QStringList)),SLOT(sl_SelectAll(QString,QStringList)));

    ivalue->show();
    this->setEnabled(true);

}

/**
 * @brief DataQueryWindow::sl_Struct_table
 */
void DataQueryWindow::sl_Struct_table()
{
    PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name;
    int i=0;

    Model = new QStandardItemModel;
    Model->setHorizontalHeaderItem(0,new QStandardItem("Field"));
    Model->setHorizontalHeaderItem(1,new QStandardItem("Type"));
    Model->setHorizontalHeaderItem(2,new QStandardItem("Null"));
    Model->setHorizontalHeaderItem(3,new QStandardItem("Key"));
    Model->setHorizontalHeaderItem(4,new QStandardItem("Default"));
    Model->setHorizontalHeaderItem(5,new QStandardItem("Extra"));

    QSqlQuery query4("show columns from "+Table_Name);
    while(query4.next())
    {
        for(int k=0;k<6;k++)
        {
            if(k==4 && query4.value(k).toString()=="")
            {
                Item = new QStandardItem ("NULL");
            }
            else
                Item = new QStandardItem (query4.value(k).toString());
            Model->setItem(i,k,Item);

            ////////// * alternate color of rows  */////////////////
            if(i%2==0)
                Model->item(i,k)->setBackground(Qt::darkGray);
            else
                Model->item(i,k)->setBackground(Qt::lightGray);
            ////////////////////////////////////////////////////////
        }
        i++;
    }
    ui->tableView_QueryData->setModel(Model);

}

/**
 * @brief DataQueryWindow::sl_Browse_table
 */
void DataQueryWindow::sl_Browse_table()
{
    PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name;
    On_Click_treeview(Mdl_Index);
}

/**
 * @brief DataQueryWindow::sl_Drop_table
 */
void DataQueryWindow::sl_Drop_table()
{
     PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name;
     QSqlQuery queryDROP;
     queryDROP.exec("drop table "+Table_Name);
     sl_TreeViewList();
     ui->tableView_QueryData->setModel(nullptr);
}

/**
 * @brief DataQueryWindow::sl_Import_table
 */
void DataQueryWindow::sl_Import_table()
{
    isExport=false;
    PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name;
    on_pushButton_Upload_clicked();
}

/**
 * @brief DataQueryWindow::sl_Export_table
 */
void DataQueryWindow::sl_Export_table()
{
    isExport=true;
    PRINT(__FUNCTION__)<<"Table_Name "<<Table_Name;
    on_pushButton_Upload_clicked();
}

/**
 * @brief DataQueryWindow::sl_About
 */
void DataQueryWindow::sl_About()
{
    QMessageBox::information(this,"About us","<b>MySQL GUI</b><br>"
                                             "Version 1.1b<br>"
                                             "<br>Copyright 2022-2023 The anil-arise1600. All rights reserved.<br>"
                                             "<br>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, "
                                             "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE");
}

void DataQueryWindow::sl_ClosedSwitchDB(bool isSwitched)
{
    if(isSwitched)
    {
        ui->label_selectedDB->setText(dba->databaseName());
        sl_TreeViewList();
        ui->tableView_QueryData->setModel(nullptr);
    }
}

/**
 * @brief DataQueryWindow::eventFilter
 * @param obj
 * @param event
 * @return
 */
bool DataQueryWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if(ui->lineEdit_Query->hasFocus() && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *KE = static_cast<QKeyEvent*>(event);
        if (KE->key() == Qt::Key_Return || KE->key() == Qt::Key_Enter)
        {
            PRINT(__FUNCTION__)<<"Pressed" <<QKeySequence(KE->key()).toString(QKeySequence::NativeText);
            on_pushButton_Go_clicked();
            return  true;
        }
    }
    return false ;

}
