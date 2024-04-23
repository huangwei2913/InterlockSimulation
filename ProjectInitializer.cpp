#include "ProjectInitializer.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

//在这里构造一个联锁控制表，
void ProjectInitializer::constructRouteTable() {
	//QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	//db.setDatabaseName(databasePath_);
	//该进路表只是记录用户在界面中，能够从那个信号机选择那个信号机，经过哪些区段，经过哪些道岔	if (db.open()) {
		QSqlQuery query(db);
		query.exec(
			"CREATE TABLE IF NOT EXISTS Routes ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"path TEXT,"
			"sections TEXT"
			")"
		);

		// 定义路径和区段信息
		QStringList paths = {
"X -> 5 -> 7 -> 9 -> SII",
"X -> 5 -> 7 -> 9 -> S4",
"X -> 5 -> 7 -> 9 -> S4 -> X4 -> 2 -> S",
"X -> 5 -> 3 -> 11 -> SI",
"X -> 5 -> 3 -> 11 -> S3",
"X -> 5 -> 3 -> 11 -> SI -> XI -> 4 -> SN",
"X -> 5 -> 3 -> 11 -> SI -> XI",
"X -> 5 -> 7 -> 9 -> SII -> XII -> 2 -> S",
"X -> 5 -> 7 -> 9 -> S4 -> X4",
"X -> 5 -> 7 -> 9 -> SII -> XII",
"X -> 5 -> 3 -> 11 -> S3 -> X3 -> 4 -> SN",
"X -> 5 -> 3 -> 11 -> S3 -> X3",
"SI -> 11 -> 3 -> 1 -> XN",
"SI -> XI -> 4 -> SN",
"SI -> 11 -> 3 -> 5 -> X",
"XI -> SI -> 11 -> 3 -> 1 -> XN",
"XI -> SI -> 11 -> 3 -> 5 -> X",
"XI -> 4 -> SN",
"SN -> 4 -> XI",
"SN -> 4 -> XI -> SI",
"SN -> 4 -> XI -> SI -> 11 -> 3 -> 5 -> X",
"SN -> 4 -> XI -> SI -> 11 -> 3 -> 1 -> XN",
"SN -> 4 -> X3 -> S3",
"SN -> 4 -> X3 -> S3 -> 11 -> 3 -> 1 -> XN",
"SN -> 4 -> X3 -> S3 -> 11 -> 3 -> 5 -> X",
"SN -> 4 -> X3",
"S -> 2 -> XII -> SII -> 9 -> S4",
"S -> 2 -> X4 -> S4 -> 9 -> 7 -> 1 -> XN",
"S -> 2 -> X4 -> S4",
"S -> 2 -> X4",
"S -> 2 -> XII",
"S -> 2 -> XII -> SII -> 9 -> 7 -> 5 -> X",
"S -> 2 -> XII -> SII -> 9 -> S4 -> X4",
"S -> 2 -> X4 -> S4 -> 9 -> 7 -> 5 -> X",
"S -> 2 -> XII -> SII",
"S -> 2 -> XII -> SII -> 9 -> 7 -> 1 -> XN",
"XII -> SII -> 9 -> S4 -> X4 -> 2 -> S",
"XII -> 2 -> S",
"XII -> SII -> 9 -> 7 -> 5 -> X",
"XII -> SII -> 9 -> 7 -> 1 -> XN",
"XII -> SII -> 9 -> S4",
"SII -> 9 -> 7 -> 5 -> X",
"SII -> 9 -> S4 -> X4 -> 2 -> S",
"SII -> XII -> 2 -> S",
"SII -> 9 -> 7 -> 1 -> XN",
"SII -> 9 -> S4 -> X4",
"XN -> 1 -> 3 -> 11 -> SI",
"XN -> 1 -> 3 -> 11 -> S3",
"XN -> 1 -> 7 -> 9 -> SII -> XII -> 2 -> S",
"XN -> 1 -> 7 -> 9 -> S4 -> X4",
"XN -> 1 -> 7 -> 9 -> S4",
"XN -> 1 -> 3 -> 11 -> S3 -> X3 -> 4 -> SN",
"XN -> 1 -> 7 -> 9 -> SII -> XII",
"XN -> 1 -> 7 -> 9 -> S4 -> X4 -> 2 -> S",
"XN -> 1 -> 3 -> 11 -> SI -> XI",
"XN -> 1 -> 3 -> 11 -> SI -> XI -> 4 -> SN",
"XN -> 1 -> 3 -> 11 -> S3 -> X3",
"XN -> 1 -> 7 -> 9 -> SII",
"S3 -> 11 -> 3 -> 5 -> X",
"S3 -> X3 -> 4 -> SN",
"S3 -> 11 -> 3 -> 1 -> XN",
"S4 -> 9 -> 7 -> 5 -> X",
"S4 -> X4 -> 2 -> S",
"S4 -> 9 -> 7 -> 1 -> XN",
"X4 -> S4 -> 9 -> 7 -> 1 -> XN",
"X4 -> S4 -> 9 -> 7 -> 5 -> X",
"X4 -> 2 -> S",
"X3 -> S3 -> 11 -> 3 -> 5 -> X",
"X3 -> 4 -> SN",
"X3 -> S3 -> 11 -> 3 -> 1 -> XN",


		};
		QStringList sections = {
"['3-11DG', '1-9DG']",
"['3-11DG', '1-9DG', '4G']",
"['3-11DG', '1-9DG', '4G', '2DG']",
"['3-11DG', 'IG']",
"['3-11DG', '3G']",
"['3-11DG', 'IG', '4DG', 'X1LQG']",
"['3-11DG', 'IG', '4DG']",
"['3-11DG', '1-9DG', 'IIG', '2DG']",
"['3-11DG', '1-9DG', '4G']",
"['3-11DG', '1-9DG', 'IIG']",
"['3-11DG', '3G', '4DG', 'X1LQG']",
"['3-11DG', '3G']",
"['IG', '3-11DG', '1-9DG', 'S1LQG']",
"['IG', '4DG', 'X1LQG']",
"['IG', '3-11DG']",
"['4DG', 'IG', '3-11DG', '1-9DG', 'S1LQG']",
"['4DG', 'IG', '3-11DG']",
"['4DG', 'X1LQG']",
"['X1LQG', '4DG']",
"['X1LQG', '4DG', 'IG']",
"['X1LQG', '4DG', 'IG', '3-11DG']",
"['X1LQG', '4DG', 'IG', '3-11DG', '1-9DG', 'S1LQG']",
"['X1LQG', '4DG', '3G']",
"['X1LQG', '4DG', '3G', '3-11DG', '1-9DG', 'S1LQG']",
"['X1LQG', '4DG', '3G', '3-11DG']",
"['X1LQG', '4DG', '3G']",
"['2DG', 'IIG', '1-9DG', '4G']",
"['2DG', '4G', '1-9DG', 'S1LQG']",
"['2DG', '4G']",
"['2DG', '4G']",
"['2DG', 'IIG']",
"['2DG', 'IIG', '1-9DG', '3-11DG']",
"['2DG', 'IIG', '1-9DG', '4G']",
"['2DG', '4G', '1-9DG', '3-11DG']",
"['2DG', 'IIG', '1-9DG']",
"['2DG', 'IIG', '1-9DG', 'S1LQG']",
"['IIG', '1-9DG', '4G', '2DG']",
"['IIG', '2DG']",
"['IIG', '1-9DG', '3-11DG']",
"['IIG', '1-9DG', 'S1LQG']",
"['IIG', '1-9DG', '4G']",
"['1-9DG', '3-11DG']",
"['1-9DG', '4G', '2DG']",
"['1-9DG', 'IIG', '2DG']",
"['1-9DG', 'S1LQG']",
"['1-9DG', '4G']",
"['S1LQG', '1-9DG', '3-11DG', 'IG']",
"['S1LQG', '1-9DG', '3-11DG', '3G']",
"['S1LQG', '1-9DG', 'IIG', '2DG']",
"['S1LQG', '1-9DG', '4G']",
"['S1LQG', '1-9DG', '4G']",
"['S1LQG', '1-9DG', '3-11DG', '3G', '4DG', 'X1LQG']",
"['S1LQG', '1-9DG', 'IIG']",
"['S1LQG', '1-9DG', '4G', '2DG']",
"['S1LQG', '1-9DG', '3-11DG', 'IG', '4DG']",
"['S1LQG', '1-9DG', '3-11DG', 'IG', '4DG', 'X1LQG']",
"['S1LQG', '1-9DG', '3-11DG', '3G']",
"['S1LQG', '1-9DG']",
"['3G', '3-11DG']",
"['3G', '4DG', 'X1LQG']",
"['3G', '3-11DG', '1-9DG', 'S1LQG']",
"['4G', '1-9DG', '3-11DG']",
"['4G', '2DG']",
"['4G', '1-9DG', 'S1LQG']",
"['4G', '1-9DG', 'S1LQG']",
"['4G', '1-9DG', '3-11DG']",
"['4G', '2DG']",
"['3G', '3-11DG']",
"['3G', '4DG', 'X1LQG']",
"['3G', '3-11DG', '1-9DG', 'S1LQG']",

		};
		// Insert the paths and sections into the Routes table
		// Insert the paths and sections into the Routes table if they do not already exist
		for (int i = 0; i < paths.size(); ++i) {
			query.prepare("SELECT COUNT(*) FROM Routes WHERE path = ?");
			query.addBindValue(paths[i]);
			query.exec();

			if (query.next() && query.value(0).toInt() == 0) {
				query.prepare("INSERT INTO Routes (path, sections) VALUES (?, ?)");
				query.addBindValue(paths[i]);
				query.addBindValue(sections[i]);
				query.exec();
			}
		}

	
}
