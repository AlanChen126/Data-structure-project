﻿#include "class_system.h"

//把json文件的数据传入courses结构体数组中
void load_json(vector<Course>& courses);

class_system::class_system(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //新建DAG对象
    vector<Course> courses;
    load_json(courses);
    dag = DAG(courses);//一定是这样写！

    //class_table表格
    ui.class_table->horizontalHeader()->setStretchLastSection(true); //设置表头充满表格的宽度
    ui.class_table->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置表格内容不可编辑
    ui.class_table->setAlternatingRowColors(true);
    //ui.www->setEditTriggers(QAbstractItemView::NoEditTriggers);//只读，双击之后无法修改值
    //ui.class_table->setItem(0, 1, new QTableWidgetItem("111"));
    //设置表头的边框
    ui.class_table->horizontalHeader()->setStyleSheet("QHeaderView::section {border: border-right: 1px solid black;}");
    ui.class_table->verticalHeader()->setStyleSheet("QHeaderView::section {border: border-right: 1px solid black;}");

    //点击按钮显示界面      连接信号和槽
    connect(ui.set, SIGNAL(clicked()), this, SLOT(set_data_Button_clicked()));//点击set按钮设置学时
    connect(ui.change_term, SIGNAL(clicked()), this, SLOT(change_term_Button_clicked()));//点击change按钮切换课程所在学期
    connect(ui.show_dag, SIGNAL(clicked()), this, SLOT(show_dag_Button_clicked()));//点击显示有向图按钮显示当前的有向图
    connect(ui.choose, SIGNAL(clicked()), this, SLOT(show_class_Button_clicked()));//点击显示课表按钮显示当前排课结果
    connect(ui.save, SIGNAL(clicked()), this, SLOT(save_Button_clicked()));//点击保存按钮保存当前课表信息
    connect(ui.fallback, SIGNAL(clicked()), this, SLOT(fallback_Button_clicked()));//点击回退按钮把排课记录返回到上一次修改
}

class_system::~class_system()
{}

//修改学时和学分
void class_system::set_data_Button_clicked()
{
    // 创建 set_data 窗口对象
    set_window* set_data_window = new set_window(this);
    set_data_window->setWindowFlags(Qt::Window);//要先设置成窗口属性
    set_data_window->show();

    // 连接信号与槽.获得学分学时数据   设置成点击按钮就启动sort，检测是否拿到数据
    connect(set_data_window, &set_window::dataReady, this, &class_system::handleDataReady);
    //后续函数在set_window中实现
}

//处理从设置学分界面得到的学分和学时的数据
void class_system::handleDataReady(vector<int> xueshi, vector<int> xuefen) {
    // 在这里调用 sort 函数，并传递 xueshi 和 xuefen 值

    dag.printAdjacencyMatrix();

    dag.sort(xueshi, xuefen);
}

//修改课程所在学期
void class_system::change_term_Button_clicked()
{
    //创建窗口对象
    change_term* change_term_window = new change_term(this);
    change_term_window->setWindowFlags(Qt::Window);//要先设置成窗口属性
    change_term_window->show();

    // 连接信号与槽.获得学分学时数据   在 changeReady 信号被触发时，自动调用 handlechangeReady 槽函数(传参数使用)
    connect(change_term_window, &change_term::changeReady, this, &class_system::handlechangeReady);
}

//展示有向无环图
void class_system::show_dag_Button_clicked()
{
    show_dag* show_dag_window = new show_dag(this);
    show_dag_window->setWindowFlags(Qt::Window);//要先设置成窗口属性
    show_dag_window->show();

}

//点击保存按钮保存当前课表信息
void class_system::save_Button_clicked()
{
    QString result,txt;
    result = dag.saveCurrentAssignment();
    txt = dag.save_txt();
    ui.debug->append(result);
    ui.debug->append(txt);
}

//点击回退按钮把排课记录返回到上一次修改
void class_system::fallback_Button_clicked()
{
    QString undo_result,txt;
    undo_result = dag.undo();
    txt = dag.removeFile("class_table.txt");
    ui.debug->append(undo_result);
    ui.debug->append(txt);
}


//调整课程对应的学期，获取用户在窗口输入的参数，传给dag函数
void class_system::handlechangeReady(int class_num, int term_num)
{
    QString result = dag.adjust_term(class_num, term_num); // 获取排序结果字符串
    ui.debug->append(result); // 将结果显示在 QTextBrowser 中，不会覆盖原先内容
}


//把json文件的数据传入courses结构体数组中
void load_json(vector<Course>& courses)
{
    // 打开JSON文件
    QFile file("课程数据.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开JSON文件.";
        return;
    }
    // 读取JSON文件的全部内容
    QByteArray jsonData = file.readAll();
    // 将JSON数据解析为JSON文档
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    // 检查JSON文档是否有效
    if (jsonDoc.isNull()) {
        qWarning() << "JSON解析失败.";
        return;
    }
    // 获取JSON对象
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray coursesArray = jsonObj["courses"].toArray();

    // 遍历JSON数组
    for (const QJsonValue& courseValue : coursesArray) {
        if (courseValue.isObject()) {
            // 获取课程对象的JSON表示
            QJsonObject courseObj = courseValue.toObject();

            QString name = courseObj["name"].toString();
            int class_num = courseObj["class_num"].toInt();

            vector<int> pre_class;
            QJsonArray preClassArray = courseObj["pre_class"].toArray();
            for (const QJsonValue& preReqValue : preClassArray) {
                if (preReqValue.isDouble()) {
                    pre_class.push_back(preReqValue.toInt());
                }
            }

            int score = courseObj["score"].toInt();
            int hours = courseObj["hours"].toInt();

            courses.push_back(Course(name.toStdString(), class_num, pre_class, score, hours));
        }
    }

}

void class_system::show_class_Button_clicked()
{
    QString result = dag.show_result(); // 获取排序结果字符串
    //ui.debug->setText(result); // 将结果显示在 QTextBrowser 中，会覆盖原先内容
    ui.debug->append(result); // 将结果显示在 QTextBrowser 中，不会覆盖原先内容
    vector<vector<string>> classAssignments = dag.createClassAssignments();//接收课程信息的二维数组
    show_table(classAssignments);//把二维数组内容放入class_table中显示
}

// 将排课结果保存在表格中
void class_system::show_table(const vector<vector<string>>& classAssignments) {
    // 确保二维数组的大小与表格大小匹配
    if (classAssignments.size() != 8 || classAssignments[0].size() != 8) {
        qDebug() << "表格大小不匹配";
        qDebug() << "classAssignments size: " << classAssignments.size() << "x" << classAssignments[0].size();
        return;
    }

    for (int i = 0; i < 8; ++i) {//表格的列
        for (int j = 0; j < 8; ++j) {//表格的行
            // 创建可编辑的表格单元格
            QString itemText = QString::fromStdString(classAssignments[i][j]);//获取对应值
            QTableWidgetItem* item = new QTableWidgetItem(itemText);//新建一个表格元素
            ui.class_table->setItem(j, i, item);//设置表格单元格
        }
    }
}

