#ifndef SHIKEN_DATE_TIME_PARSER_H
#define SHIKEN_DATE_TIME_PARSER_H

#include <shiken/config.h>
#include <QDateTime>
#include "xsd_DateTime.h"

namespace shiken {

/**
 * Implementation of a parser for QDateTime class.
 */
class DateTimeParser: public xsd::shiken::DateTime_pskel {
public:
    virtual void year(int year) override {
        m_year = year;
    }

    virtual void month(int month) override {
        m_month = month;
    }

    virtual void mday(int mday) override {
        m_mday = mday;
    }

    virtual void hour(int hour) override {
        m_hour = hour;
    }

    virtual void min(int min) override {
        m_min = min;
    }

    virtual void sec(int sec) override {
        m_sec = sec;
    }

    virtual void zone(int zone) override {
        m_zone = zone;
    }

    virtual void dst(int dst) override {
        m_dst = dst;
    }

    virtual QDateTime post_DateTime() override {
        QDateTime result;
        result.setDate(QDate(m_year, m_month, m_mday));
        result.setTime(QTime(m_hour, m_min, m_sec));
        result.setUtcOffset((m_zone + m_dst) / 1000);
        return result;
    }

private:
    int m_year, m_month, m_mday, m_hour, m_min, m_sec, m_zone, m_dst;
};

} // namespace shiken

#endif // SHIKEN_Q_DATE_TIME_XML_H
