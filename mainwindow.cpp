#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setResizeMode(QHeaderView::Stretch);
    makeDBConnection();
    ui->keyboardFrame->setHidden(true);
    initAlphabet();

    on_ArButton_clicked(); // Load Arabic by default
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeDBConnection()
{
    QString path = "data/univlexique.db";
    if(QFile::exists(path))
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);
        if(!db.open())
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open database!"));
            return;
        }
    }
}

QString MainWindow::generateHeader()
{
    QString html =
            tr("<html>") +
            "<head>"
                " <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"
                "<title>[Nibras]</title>"
            "</head>"
            "<body >"
                "<div id=\"header\">"
                    "<p align=\"center\">"
                    + tr("Nibras search result for the term: ") + ui->searchLineEdit->text() +
                    " </p>"
                    "<br />"
                    "<br />"
                "</div>"
            " </body>"
            "</html>";
    return html;
}

void MainWindow::generateBody()
{
    document = new QTextDocument(this);
    QTextCursor cursor(document);
    cursor.movePosition(QTextCursor::Start);


    QString column1Title = ui->tableWidget->horizontalHeaderItem(0)->text();
    QString column2Title = ui->tableWidget->horizontalHeaderItem(1)->text();

    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignHCenter);
    tableFormat.setCellPadding(5);
    tableFormat.setCellSpacing(0);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    QBrush brush(Qt::SolidPattern);
    brush.setColor(Qt::black);
    tableFormat.setBorderBrush(brush);
    tableFormat.setBorder(1);

    QVector<QTextLength> columnWidth;
    columnWidth.append(QTextLength(QTextLength::PercentageLength, 50));
    columnWidth.append(QTextLength(QTextLength::PercentageLength, 50));

    tableFormat.setColumnWidthConstraints(columnWidth);

    QTextTable *tTable = cursor.insertTable(2, 2);

    tTable->setFormat(tableFormat);

    QTextBlockFormat centerAlignment;
    centerAlignment.setAlignment(Qt::AlignHCenter);

    QTextBlockFormat leftAlignment;
    leftAlignment.setAlignment(Qt::AlignLeft);

    QTextBlockFormat rightAlignment;
    rightAlignment.setAlignment(Qt::AlignRight);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);

    cursor.setBlockFormat(centerAlignment);
    cursor.insertText(column1Title, boldFormat);
    cursor.movePosition(QTextCursor::NextCell);

    cursor.setBlockFormat(centerAlignment);
    cursor.insertText(column2Title, boldFormat);


    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
    {

        tTable->appendRows(1);
        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(leftAlignment);
        cursor.insertText(ui->tableWidget->item(i,0)->text());

        cursor.movePosition(QTextCursor::NextCell);
        cursor.setBlockFormat(leftAlignment);
        cursor.insertText(ui->tableWidget->item(i,1)->text());
    }

    tTable->removeRows(tTable->rows()-1, 1);
    cursor.movePosition(QTextCursor::End);
}


void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

void MainWindow::clearTable()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void MainWindow::on_searchLineEdit_textChanged()
{
    on_searchButton_clicked();
}

void MainWindow::on_searchButton_clicked()
{
    clearTable();

    QString arg = ui->searchLineEdit->text().trimmed();

    if(arg.isEmpty()) return;

    // don't change these lines unless you ensure that this file is encoded with UTF-8
    QString fatha = QString::fromUtf8("َ");
    QString damma = QString::fromUtf8("ُ");
    QString fathatin = QString::fromUtf8("ً");
    QString dammatin = QString::fromUtf8("ٌ");
    QString kasra = QString::fromUtf8("ِ");
    QString kasratin = QString::fromUtf8("ٍ");
    QString sukun = QString::fromUtf8("ْ");

    // remove tashkeel
    arg = arg.remove(fatha);
    arg.remove(damma);
    arg.remove(fatha);
    arg.remove(fathatin);
    arg.remove(dammatin);
    arg.remove(kasra);
    arg.remove(kasratin);
    arg.remove(sukun);

    QSqlQuery query;
    if(alphabetQuery) // have to be true when the user clicks on one of the virtual keyboard alphabet button
    {
        // if the user clicks a specific alphabet button, the Query has to show all words that *begin* with
        // the clicked button's letter

        query.exec(QString("SELECT FRENCH, ARABIC FROM univ "
                        "WHERE FRENCH LIKE \"%1%\" OR ARABIC LIKE \"%1%\" COLLATE NOCASE").arg(arg));

        alphabetQuery = false; // back to normal mode
    }
    else
    {
        // the query here shows all words that *contains* the arg

        query.exec(QString("SELECT FRENCH, ARABIC FROM univ "
                        "WHERE FRENCH LIKE \"%%1%\" OR ARABIC LIKE \"%%1%\" COLLATE NOCASE").arg(arg));
    }
    int i = 0;

    // in case the user pressed on zoom in/out let's keep the same rows height
    QFont font = ui->tableWidget->font();

    while(query.next())
    {

        ui->tableWidget->insertRow(i);
        // keep the row heigth to fit the font size
        ui->tableWidget->setRowHeight(i, font.pointSize() * 3);

        ui->tableWidget->setItem(i, 0, new QTableWidgetItem());
        ui->tableWidget->item(i, 0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->tableWidget->item(i, 0)->setTextAlignment(Qt::AlignVCenter|Qt::AlignLeft);
        ui->tableWidget->item(i, 0)->setText(query.value(0).toString());

        ui->tableWidget->setItem(i, 1, new QTableWidgetItem());
        ui->tableWidget->item(i, 1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->tableWidget->item(i, 1)->setTextAlignment(Qt::AlignVCenter|Qt::AlignRight);
        ui->tableWidget->item(i, 1)->setText(query.value(1).toString());

        i++;
    }
}


void MainWindow::on_actionZoom_in_triggered()
{
    QFont font = ui->tableWidget->font();
    font.setPointSize(font.pointSize() + 1);
    ui->tableWidget->setFont(font);

    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
        ui->tableWidget->setRowHeight(i, font.pointSize() * 3);
}


void MainWindow::on_actionZoom_out_triggered()
{
    QFont font = ui->tableWidget->font();
    font.setPointSize(font.pointSize() - 1);
    ui->tableWidget->setFont(font);

    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
        ui->tableWidget->setRowHeight(i, font.pointSize() * 3);
}


void MainWindow::on_actionCheck_for_update_triggered()
{
    Updates up;
    up.exec();
}

void MainWindow::on_actionFonts_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(
                &ok, ui->tableWidget->font(), this);
    if (ok) {
        ui->tableWidget->setFont(font);
        for(int i = 0; i < ui->tableWidget->rowCount(); i++)
            ui->tableWidget->setRowHeight(i, font.pointSize() * 3);
    }
}

void MainWindow::on_actionSuggest_New_triggered()
{
    QDesktopServices::openUrl(QUrl("http://nibras.sourceforge.net/"));
}

void MainWindow::setButtonContentToSearch(QAbstractButton *button)
{
    // The query concerns alphabet, it has to show results that *start* with the given letter
    alphabetQuery = true;

    /*
        Clear the search field to ensure that a "text changed" signal will be emitted
        even when the old content is the same as the new one.
    */
    ui->searchLineEdit->clear();

    // set the button text as the search line edit content, this will perform a search as the text is changed.
    ui->searchLineEdit->setText(button->text());
}

void MainWindow::initAlphabet()
{
    // Add all buttons inside the virtual keyboard frame
    QList<QPushButton *> buttonsList = ui->keyboardFrame->findChildren<QPushButton *>();
    foreach (QPushButton *button, buttonsList) {
        buttonGroup.addButton(button);
    }

    // The alphabet is hidden by default
    alphabetIsShown = false;

    // the search query doesn't concern alphabets for the first time
    alphabetQuery = false;

    // Catch every alphabet button "clicked" signals
    connect(&buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(setButtonContentToSearch(QAbstractButton*)));
}

void MainWindow::on_showAlphabetButton_clicked()
{
    if(alphabetIsShown) // then hide it
    {
        ui->keyboardFrame->setHidden(true);
        ui->showAlphabetButton->setText(tr("Show alphabet +"));
        alphabetIsShown = false;
    }
    else // then show it
    {
        ui->keyboardFrame->setHidden(false);
        ui->showAlphabetButton->setText(tr("Hide alphabet -"));
        alphabetIsShown= true;
    }
}

void MainWindow::on_actionCopy_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString terms;
    int rows = ui->tableWidget->selectedItems().count();

    // if the user has selected specific cells, then only copy selected cells
    if(rows != 0)
    {
        for(int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            if(ui->tableWidget->item(i, 0)->isSelected() ||
               ui->tableWidget->item(i, 1)->isSelected())
            {
                terms += ui->tableWidget->item(i, 0)->text();
                terms += "    " + ui->tableWidget->item(i, 1)->text() + "\r\n";
            }
        }
    }
    else // copy the whole table
    {
        for(int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            terms += ui->tableWidget->item(i, 0)->text();
            terms += "    ";
            terms += ui->tableWidget->item(i, 1)->text();
            terms += "\r\n";
        }
    }

    clipboard->setText(terms);
}


void MainWindow::on_actionPrint_triggered()
{
    textPrinter = new TextPrinter(this);
    textPrinter->setHeaderSize(20);
    textPrinter->setFooterSize(19);
    textPrinter->setSpacing(0);

    textPrinter->setPageSize(QPrinter::A4);
    textPrinter->setLeftMargin(3);
    textPrinter->setTopMargin(2);
    textPrinter->setRightMargin(7);
    textPrinter->setBottomMargin(5);
    textPrinter->pageSize();
    generateBody();

    textPrinter->setHeaderText(generateHeader());
    textPrinter->setFooterText(generateFooter());
    textPrinter->print(document, tr("Preview before printing of the search result for the term: ")
                         + ui->searchLineEdit->text().trimmed());
}


QString MainWindow::generateFooter()
{
    QString html =
            "<html>"
            "<head></head>"
            "<body>"
                "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
                "<tr>"
                    "<td><p align=\"left\">" + QDate::currentDate().toString("ddd MMMM d yy") + "</p></td>"
                    "<td><p align=\"right\">" + tr("Page: ") + "&page;" + "</p></td>"
                "</tr>"
                "</table>"
            "</body>"
            "</html>";
    return html;
}

void MainWindow::on_actionExport_To_HTML_triggered()
{

    QString path = QFileDialog::getSaveFileName(this, tr("Export to HTML"), tr("[Nibras] ") +
                                                ui->searchLineEdit->text().trimmed(), "HTML (*.html)");
    if(!path.isEmpty())
    {
        // header
        QString html =
                tr("<html>") +
                "<head>"
                    " <meta http-equiv=\"Content-Type\" content=\"text/html; charset=Windows-1256\" />"
                "<title>[Nibras]</title>"
                "</head>"
                "<body >"
                    "<div id=\"header\">"
                        "<p align=\"center\">"
                        + tr("Nibras search result for the term: ") + ui->searchLineEdit->text() +
                        " </p>"
                        "<br />"
                        "<br />"
                    "</div>";

        //body
        generateBody();
        QString body = document->toHtml();
        body.remove(0, body.indexOf("<table"));
        body.remove(body.indexOf("</body>"), body.length() - body.indexOf("</html>") + 7);
        html += body.toUtf8();

        // footer
        html += "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">"
                    "<tr>"
                        "<td><p align=\"left\">" + QDate::currentDate().toString("ddd MMMM d yy") + "</p></td>"
                    "</tr>"
                    "</table>"
                "</body>"
                "</html>";

        QFile file(path);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << html;
    }
}


void MainWindow::loadLanguage(const QString &rLanguage)
{
    if(m_currLang != rLanguage)
        {
            m_currLang = rLanguage;

            QLocale locale = QLocale(m_currLang);
            QLocale::setDefault(locale);
            switchTranslator(m_translator, QString("Daleel_%1.qm").arg(rLanguage));
            switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));
    }
}

void MainWindow::switchTranslator(QTranslator &translator, const QString &filename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    if(translator.load(filename))
        qApp->installTranslator(&translator);
}

void MainWindow::changeEvent(QEvent *event)
{
    if(0 != event)
    {
        if(event->type() == QEvent::LanguageChange)
        {
            // this event is send if a translator is loaded
            ui->retranslateUi(this);
        }
    }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if(ui->tableWidget->underMouse() && ui->tableWidget->rowCount() != 0)
    {
        QMenu context(ui->tableWidget);

        QAction* copyAction = new QAction(tr("Copy"), &context);
        connect(copyAction, SIGNAL(triggered()), this, SLOT(copyCell()));

        QAction* copyRowAction = new QAction(tr("Copy Row"), &context);
        connect(copyRowAction, SIGNAL(triggered()), this, SLOT(copyRow()));

        QAction* copyTable = new QAction(tr("Copy the whole table"), &context);
        connect(copyTable, SIGNAL(triggered()), this, SLOT(copyTable()));

        context.addAction(copyAction);
        context.addAction(copyRowAction);
        context.addSeparator();
        context.addAction(copyTable);
        context.exec(event->globalPos());
    }
}

void MainWindow::on_FRButton_clicked()
{
    loadLanguage("fr");
    qApp->setLayoutDirection(Qt::LeftToRight);
    this->addToolBar(Qt::LeftToolBarArea, ui->mainToolBar);

    ui->baseFrame->setStyleSheet("#baseFrame {"
                                 "background: #414141;"
                                 "border-left: 1px solid #4b4b4b;}");
}

void MainWindow::on_ArButton_clicked()
{
    loadLanguage("ar");
    qApp->setLayoutDirection(Qt::RightToLeft);
    this->addToolBar(Qt::RightToolBarArea, ui->mainToolBar);
    ui->baseFrame->setStyleSheet("#baseFrame {"
                                 "background: #414141;"
                                 "border-right: 1px solid #4b4b4b;}");

    // keep the layout direction of the following widgets to LTR
    ui->keyboardFrame->setLayoutDirection(Qt::LeftToRight);
    ui->tableWidget->setLayoutDirection(Qt::LeftToRight);
}

void MainWindow::on_actionPrintPreview_triggered()
{
    textPrinter = new TextPrinter(this);
    textPrinter->setHeaderSize(20);
    textPrinter->setFooterSize(19);
    textPrinter->setSpacing(0);

    textPrinter->setPageSize(QPrinter::A4);
    textPrinter->setLeftMargin(3);
    textPrinter->setTopMargin(2);
    textPrinter->setRightMargin(7);
    textPrinter->setBottomMargin(5);
    textPrinter->pageSize();
    generateBody();

    textPrinter->setHeaderText(generateHeader());
    textPrinter->setFooterText(generateFooter());
    textPrinter->preview(document, tr("Preview before printing of the search result for the term: ")
                         + ui->searchLineEdit->text().trimmed());
}

void MainWindow::on_actionAbout_the_program_triggered()
{
    About about;
    about.exec();
}

void MainWindow::copyCell()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->tableWidget->currentItem()->text());
}

void MainWindow::copyRow()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString terms;

    int i = ui->tableWidget->currentItem()->row();
    terms += ui->tableWidget->item(i, 0)->text();
    terms += "    " + ui->tableWidget->item(i, 1)->text() + "\r\n";

    clipboard->setText(terms);
}

void MainWindow::copyTable()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString terms;

    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        terms += ui->tableWidget->item(i, 0)->text();
        terms += "    ";
        terms += ui->tableWidget->item(i, 1)->text();
        terms += "\r\n";
    }

    clipboard->setText(terms);
}
